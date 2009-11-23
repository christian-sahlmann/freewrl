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
	int	depth;
	int 	hasAlpha;
	int	OpenGLTexture;
	int	frames;
	char    *filename;
        int x;
        int y;
        unsigned char *texdata;
	char *pixelData;
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
	if(!loaded)
	{
		initImageLoader();
		loaded = 1;
	}
	// convert to wide char http://msdn.microsoft.com/en-us/library/ms235631(VS.80).aspx   
    size_t origsize = strlen(fname) + 1;
    const size_t newsize = 100;
    size_t convertedChars = 0;
    wchar_t wcstring[newsize];
    mbstowcs_s(&convertedChars, wcstring, origsize, fname, _TRUNCATE);
    tti->filename = fname;
    tti->status = TEX_NEEDSBINDING;

   Bitmap* bitmap = new Bitmap(wcstring); //L"LockBitsTest1.bmp");
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
   Rect rect(0,0,bitmap->GetWidth(),bitmap->GetHeight());

   // Lock a 5x3 rectangular portion of the bitmap for reading.
   bitmap->LockBits(
      &rect,
      ImageLockModeRead,
	  PixelFormat32bppARGB, //PixelFormat24bppRGB, 
      bitmapData);

#ifdef verbose
   printf("The stride is %d.\n\n", bitmapData->Stride);
   printf("bitmapData W=%d H=%d\n",bitmapData->Width,bitmapData->Height);
#endif

   // Display the hexadecimal value of each pixel in the 5x3 rectangle.
   UINT* pixels = (UINT*)bitmapData->Scan0;

#ifdef verbose
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
   tti->depth = 4;
   tti->frames = 1;
   int totalbytes = tti->x * tti->y * tti->depth;
   //tti->texdata = (unsigned char*)malloc(bitmapData->Width * bitmapData->Height * tti->depth);
   tti->texdata = (unsigned char*)malloc(totalbytes); //tti->x * tti->y * tti->depth);
   memcpy(tti->texdata,pixels,totalbytes); //bitmapData->Width * bitmapData->Height * tti->depth);
   tti->hasAlpha = Gdiplus::IsAlphaPixelFormat(bitmapData->PixelFormat)?1:0; //bitmap->GetPixelFormat());


   for(UINT row = 0; row < 23; ++row)
   {
      for(UINT col = 0; col < 5; ++col)
      {
         //printf("%x\n", *(UINT*)&(tti->texdata[(row * bitmapData->Stride / 4 + col)*tti->depth]));
         printf("%x\n", *(UINT*)&(tti->texdata[(row * tti->x + col)*tti->depth]));
      }
      printf("- - - - - - - - - - \n");
   }

   bitmap->UnlockBits(bitmapData);
   delete bitmapData;
   delete bitmap;
   GdiplusShutdown(gdiplusToken);

   return 0;

}
}