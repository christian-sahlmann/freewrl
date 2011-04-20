

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 6.00.0361 */
/* at Sun Apr 17 16:48:08 2011
 */
/* Compiler settings for .\freeWRLAx.idl:
    Oicf, W1, Zp8, env=Win32 (32b run)
    protocol : dce , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
//@@MIDL_FILE_HEADING(  )

#pragma warning( disable: 4049 )  /* more than 64k source lines */


/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 475
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif // __RPCNDR_H_VERSION__


#ifndef __freeWRLAxidl_h__
#define __freeWRLAxidl_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef ___DfreeWRLAx_FWD_DEFINED__
#define ___DfreeWRLAx_FWD_DEFINED__
typedef interface _DfreeWRLAx _DfreeWRLAx;
#endif 	/* ___DfreeWRLAx_FWD_DEFINED__ */


#ifndef ___DfreeWRLAxEvents_FWD_DEFINED__
#define ___DfreeWRLAxEvents_FWD_DEFINED__
typedef interface _DfreeWRLAxEvents _DfreeWRLAxEvents;
#endif 	/* ___DfreeWRLAxEvents_FWD_DEFINED__ */


#ifndef __freeWRLAx_FWD_DEFINED__
#define __freeWRLAx_FWD_DEFINED__

#ifdef __cplusplus
typedef class freeWRLAx freeWRLAx;
#else
typedef struct freeWRLAx freeWRLAx;
#endif /* __cplusplus */

#endif 	/* __freeWRLAx_FWD_DEFINED__ */


#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 


#ifndef __freeWRLAxLib_LIBRARY_DEFINED__
#define __freeWRLAxLib_LIBRARY_DEFINED__

/* library freeWRLAxLib */
/* [control][helpstring][helpfile][version][uuid] */ 


EXTERN_C const IID LIBID_freeWRLAxLib;

#ifndef ___DfreeWRLAx_DISPINTERFACE_DEFINED__
#define ___DfreeWRLAx_DISPINTERFACE_DEFINED__

/* dispinterface _DfreeWRLAx */
/* [helpstring][uuid] */ 


EXTERN_C const IID DIID__DfreeWRLAx;

#if defined(__cplusplus) && !defined(CINTERFACE)

    MIDL_INTERFACE("C3C32307-89C2-4C1A-B7F1-D561AA0653A3")
    _DfreeWRLAx : public IDispatch
    {
    };
    
#else 	/* C style interface */

    typedef struct _DfreeWRLAxVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            _DfreeWRLAx * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            _DfreeWRLAx * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            _DfreeWRLAx * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            _DfreeWRLAx * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            _DfreeWRLAx * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            _DfreeWRLAx * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            _DfreeWRLAx * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        END_INTERFACE
    } _DfreeWRLAxVtbl;

    interface _DfreeWRLAx
    {
        CONST_VTBL struct _DfreeWRLAxVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define _DfreeWRLAx_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define _DfreeWRLAx_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define _DfreeWRLAx_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define _DfreeWRLAx_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define _DfreeWRLAx_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define _DfreeWRLAx_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define _DfreeWRLAx_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)

#endif /* COBJMACROS */


#endif 	/* C style interface */


#endif 	/* ___DfreeWRLAx_DISPINTERFACE_DEFINED__ */


#ifndef ___DfreeWRLAxEvents_DISPINTERFACE_DEFINED__
#define ___DfreeWRLAxEvents_DISPINTERFACE_DEFINED__

/* dispinterface _DfreeWRLAxEvents */
/* [helpstring][uuid] */ 


EXTERN_C const IID DIID__DfreeWRLAxEvents;

#if defined(__cplusplus) && !defined(CINTERFACE)

    MIDL_INTERFACE("91DEB0AA-9E7D-43F4-80C4-125C535B30EC")
    _DfreeWRLAxEvents : public IDispatch
    {
    };
    
#else 	/* C style interface */

    typedef struct _DfreeWRLAxEventsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            _DfreeWRLAxEvents * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            _DfreeWRLAxEvents * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            _DfreeWRLAxEvents * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            _DfreeWRLAxEvents * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            _DfreeWRLAxEvents * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            _DfreeWRLAxEvents * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            _DfreeWRLAxEvents * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        END_INTERFACE
    } _DfreeWRLAxEventsVtbl;

    interface _DfreeWRLAxEvents
    {
        CONST_VTBL struct _DfreeWRLAxEventsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define _DfreeWRLAxEvents_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define _DfreeWRLAxEvents_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define _DfreeWRLAxEvents_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define _DfreeWRLAxEvents_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define _DfreeWRLAxEvents_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define _DfreeWRLAxEvents_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define _DfreeWRLAxEvents_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)

#endif /* COBJMACROS */


#endif 	/* C style interface */


#endif 	/* ___DfreeWRLAxEvents_DISPINTERFACE_DEFINED__ */


EXTERN_C const CLSID CLSID_freeWRLAx;

#ifdef __cplusplus

class DECLSPEC_UUID("582C9301-A2C8-45FC-831B-654DE7F3AF11")
freeWRLAx;
#endif
#endif /* __freeWRLAxLib_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


