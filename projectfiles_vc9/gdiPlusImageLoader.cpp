#include <windows.h>
#include <gdiplus.h>
#include <stdio.h>
#include <string.h>
using namespace Gdiplus;

extern "C"
{
#include "gdiPlusImageLoader.h"
#include <GL/glew.h>
#include "opengl/textures.h"
/*
struct textureTableIndexStruct {
	void*	scenegraphNode;
	int			nodeType;
	int	imageType;
	int 	status;
	int 	hasAlpha;
	int	OpenGLTexture;
	int	frames;
	char    *filename;
        int x;
        int y;
        unsigned char *texdata;
	// JAS char *pixelData; 
        int Src;
        int Trc;
};
*/
static ULONG_PTR gdiplusToken;
static int loaded = 0;
int initImageLoader()
{
   GdiplusStartupInput gdiplusStartupInput;
   //ULONG_PTR gdiplusToken;
   GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
	return 0;
}
int shutdownImageLoader()
{
   GdiplusShutdown(gdiplusToken);
	return 0;
}
int loadImage(struct textureTableIndexStruct *tti, char *fname)
{
	/* http://msdn.microsoft.com/en-us/library/ms536298(VS.85).aspx   GDI+ Lockbits example - what this function is based on*/
	/* http://www.microsoft.com/downloads/details.aspx?FamilyID=6a63ab9c-df12-4d41-933c-be590feaa05a&DisplayLang=en  GDI+ redistributable download - gdiplus.dll 2MB */
	if(!loaded)
	{
		initImageLoader();
		loaded = 1;
	}
	// convert to wide char http://msdn.microsoft.com/en-us/library/ms235631(VS.80).aspx   
	//fname = "C:/source2/freewrl/freex3d/tests/helpers/brick.png";  
    //fname = "junk.jpg"; //test failure condition
	size_t origsize = strlen(fname) + 1;
	char* fname2 = (char*) malloc(origsize);
	strcpy(fname2,fname);
	for(int jj=0;jj<strlen(fname2);jj++)
		if(fname2[jj] == '/' ) fname2[jj] = '\\';

    const size_t newsize = 225;
    size_t convertedChars = 0;
    wchar_t wcstring[newsize];
    //mbstowcs_s(&convertedChars, wcstring, origsize, fname, _TRUNCATE);
    mbstowcs_s(&convertedChars, wcstring, origsize, fname2, _TRUNCATE);
	free(fname2);
	Bitmap *bitmap = NULL;
	Status stat;
	bitmap = Bitmap::FromFile(wcstring,false); //new Bitmap(wcstring); //or Bitmap::FromFile(wcstring,false); L"LockBitsTest1.bmp");
	// verifying the success of constructors http://msdn.microsoft.com/en-us/library/ms533801(VS.85).aspx

	stat = bitmap->GetLastStatus(); // http://msdn.microsoft.com/en-us/library/ms535410(VS.85).aspx
	if(stat != Ok)
		return 0; //should come here if it can't find the image file
   BitmapData* bitmapData = new BitmapData;

//#define verbose 1
#ifdef verbose
   printf("bitmap W=%d H=%d\n",bitmap->GetWidth(),bitmap->GetHeight());
   printf("bitmap format index =%d\n",bitmap->GetPixelFormat()%256);
   if(Gdiplus::IsAlphaPixelFormat(bitmap->GetPixelFormat()) )
	   printf("has alpha channel\n");
   else
	   printf("no alpha channel\n");
   if(Gdiplus::IsCanonicalPixelFormat(bitmap->GetPixelFormat()) )
	   printf("is canonical\n");
   else
	   printf("not canonical\n");
   printf("Number of bits per pixel %d\n",Gdiplus::GetPixelFormatSize(bitmap->GetPixelFormat()));
#endif
   bool flipVertically = true;
   Rect rect(0,0,bitmap->GetWidth(),bitmap->GetHeight());
   if(flipVertically)
		bitmapData->Stride = -bitmap->GetWidth()*4;
   else
	   bitmapData->Stride = bitmap->GetWidth()*4;
   bitmapData->Width = bitmap->GetWidth();
   bitmapData->Height = bitmap->GetHeight();
   bitmapData->PixelFormat = PixelFormat32bppARGB;
   int totalbytes = bitmap->GetWidth() * bitmap->GetHeight() * 4; //tti->depth;
   unsigned char * blob = (unsigned char*)malloc(totalbytes);
   if(flipVertically)
		bitmapData->Scan0 = &blob[bitmap->GetWidth()*bitmap->GetHeight()*4 + bitmapData->Stride]; 
   else
	   bitmapData->Scan0 = blob;

   // Lock a rectangular portion of the bitmap for reading.
   bitmap->LockBits(
      &rect,
      ImageLockModeRead|ImageLockModeUserInputBuf,
	  PixelFormat32bppARGB, //PixelFormat24bppRGB, 
      bitmapData);

#ifdef verbose
   printf("The stride is %d.\n\n", bitmapData->Stride);
   printf("bitmapData W=%d H=%d\n",bitmapData->Width,bitmapData->Height);
#endif
#ifdef verbose

   // Display the hexadecimal value of each pixel in the 5x3 rectangle.
   UINT* pixels = (UINT*)bitmapData->Scan0;

   for(UINT row = 0; row < 23; ++row)
   {
      for(UINT col = 0; col < 5; ++col)
      {
         printf("%x\n", pixels[row * bitmapData->Stride / 4 + col]);
      }
      printf("- - - - - - - - - - \n");
   }
#endif

   //deep copy data so browser owns it (and does its FREE_IF_NZ) and we can delete our copy here and forget about it
   tti->x = bitmapData->Width;
   tti->y = bitmapData->Height;
   tti->frames = 1;
   tti->texdata = blob; 
   //tti->hasAlpha = Gdiplus::IsAlphaPixelFormat(bitmapData->PixelFormat)?1:0; 
   tti->hasAlpha = Gdiplus::IsAlphaPixelFormat(bitmap->GetPixelFormat())?1:0; 
   printf("fname=%s alpha=%ld\n",fname,tti->hasAlpha);

#ifdef verbose
   for(UINT row = 0; row < 23; ++row)
   {
      for(UINT col = 0; col < 5; ++col)
      {
         //printf("%x\n", *(UINT*)&(tti->texdata[(row * bitmapData->Stride / 4 + col)*tti->depth]));
         printf("%x\n", *(UINT*)&(tti->texdata[(row * tti->x + col)*4])); //tti->depth]));
      }
      printf("- - - - - - - - - - \n");
   }
#endif

   tti->filename = fname;
   tti->status = TEX_NEEDSBINDING; //make this the last thing you set, because another thread is watching ready to bind

   bitmap->UnlockBits(bitmapData);
   delete bitmapData;
   delete bitmap;
   //shutdownImageLoader();  //we'll keep it loaded
   if(1)
   {
	   shutdownImageLoader();
	   loaded = 0;
   }

   return 1;

}
}
