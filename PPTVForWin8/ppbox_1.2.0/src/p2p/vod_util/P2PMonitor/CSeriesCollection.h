// Machine generated IDispatch wrapper class(es) created with Add Class from Typelib Wizard

//#import "C:\\Program Files\\Microsoft Office\\Office12\\EXCEL.EXE" no_namespace
// CSeriesCollection wrapper class
#ifndef _CSeriesCollection_H
#define _CSeriesCollection_H


#include <afxdisp.h>
#import "C:\\Program Files\\Common Files\\Microsoft Shared\\OFFICE14\\mso.dll" rename("RGB", "MSRGB") rename("DocumentProperties", "JOEDocumentProperties")
#import "C:\\Program Files\\Common Files\\Microsoft Shared\\VBA\\VBA6\\VBE6EXT.OLB" raw_interfaces_only, rename("Reference", "ignorethis"), rename("VBE", "JOEVBE")
#import "C:\\Program Files\\Microsoft Office\\OFFICE14\\excel.exe" exclude("IFont", "IPicture") rename("RGB", "ignorethis"), rename("DialogBox", "ignorethis"), rename("VBE", "JOEVBE"), rename("ReplaceText", "JOEReplaceText"), rename("CopyFile","JOECopyFile"), rename("FindText", "JOEFindText"), rename("NoPrompt", "JOENoPrompt")
using namespace Office;
using namespace VBIDE;
using namespace Excel;

class CSeriesCollection : public COleDispatchDriver
{
public:
	CSeriesCollection(){} // Calls COleDispatchDriver default constructor
	CSeriesCollection(LPDISPATCH pDispatch) : COleDispatchDriver(pDispatch) {}
	CSeriesCollection(const CSeriesCollection& dispatchSrc) : COleDispatchDriver(dispatchSrc) {}

	// Attributes
public:

	// Operations
public:


	// SeriesCollection methods
public:
	LPDISPATCH get_Application()
	{
		LPDISPATCH result;
		InvokeHelper(0x94, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, NULL);
		return result;
	}
	long get_Creator()
	{
		long result;
		InvokeHelper(0x95, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
		return result;
	}
	LPDISPATCH get_Parent()
	{
		LPDISPATCH result;
		InvokeHelper(0x96, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&result, NULL);
		return result;
	}
	LPDISPATCH Add(VARIANT& Source, long Rowcol, VARIANT& SeriesLabels, VARIANT& CategoryLabels, VARIANT& Replace)
	{
		LPDISPATCH result;
		static BYTE parms[] = VTS_VARIANT VTS_I4 VTS_VARIANT VTS_VARIANT VTS_VARIANT ;
		InvokeHelper(0xb5, DISPATCH_METHOD, VT_DISPATCH, (void*)&result, parms, &Source, Rowcol, &SeriesLabels, &CategoryLabels, &Replace);
		return result;
	}
	long get_Count()
	{
		long result;
		InvokeHelper(0x76, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
		return result;
	}
	VARIANT Extend(VARIANT& Source, VARIANT& Rowcol, VARIANT& CategoryLabels)
	{
		VARIANT result;
		static BYTE parms[] = VTS_VARIANT VTS_VARIANT VTS_VARIANT ;
		InvokeHelper(0xe3, DISPATCH_METHOD, VT_VARIANT, (void*)&result, parms, &Source, &Rowcol, &CategoryLabels);
		return result;
	}
	LPDISPATCH Item(VARIANT& Index)
	{
		LPDISPATCH result;
		static BYTE parms[] = VTS_VARIANT ;
		InvokeHelper(0xaa, DISPATCH_METHOD, VT_DISPATCH, (void*)&result, parms, &Index);
		return result;
	}
	LPUNKNOWN _NewEnum()
	{
		LPUNKNOWN result;
		InvokeHelper(0xfffffffc, DISPATCH_METHOD, VT_UNKNOWN, (void*)&result, NULL);
		return result;
	}
	VARIANT Paste(long Rowcol, VARIANT& SeriesLabels, VARIANT& CategoryLabels, VARIANT& Replace, VARIANT& NewSeries)
	{
		VARIANT result;
		static BYTE parms[] = VTS_I4 VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT ;
		InvokeHelper(0xd3, DISPATCH_METHOD, VT_VARIANT, (void*)&result, parms, Rowcol, &SeriesLabels, &CategoryLabels, &Replace, &NewSeries);
		return result;
	}
	LPDISPATCH NewSeries()
	{
		LPDISPATCH result;
		InvokeHelper(0x45d, DISPATCH_METHOD, VT_DISPATCH, (void*)&result, NULL);
		return result;
	}
	LPDISPATCH _Default(VARIANT& Index)
	{
		LPDISPATCH result;
		static BYTE parms[] = VTS_VARIANT ;
		InvokeHelper(0x0, DISPATCH_METHOD, VT_DISPATCH, (void*)&result, parms, &Index);
		return result;
	}

	// SeriesCollection properties
public:

};
#endif
