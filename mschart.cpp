/*
   +----------------------------------------------------------------------+
   | PHP Version 5                                                        |
   +----------------------------------------------------------------------+
   | Copyright (c) 1997-2010 The PHP Group                                |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Authors: Pierre Joye <pierre@php.net>                                |
   +----------------------------------------------------------------------+
 */

/* $Id$ */
#include "php_mschart.h"
#include "zend_exceptions.h"

#using <mscorlib.dll>
#using <System.dll>
#using <System.Drawing.dll>
#using <System.Xml.dll>
#using <System.Windows.Forms.dll>
#using <System.Windows.Forms.DataVisualization.dll>

using namespace System;
using namespace System::Drawing;
using namespace System::IO;
using namespace System::Windows::Forms::DataVisualization::Charting;

ZEND_GET_MODULE(mschart)

PHP_FUNCTION(mschart_loadxml);

ZEND_BEGIN_ARG_INFO_EX(arginfo_mschart_loadxml, 0, 0, 1)
	ZEND_ARG_INFO(0, xmlstr)
ZEND_END_ARG_INFO()

zend_function_entry mschart_functions[] = {
    PHP_FE(mschart_loadxml, arginfo_mschart_loadxml)
    {NULL, NULL, NULL}
};

zend_module_entry mschart_module_entry = 
{
	STANDARD_MODULE_HEADER,
	"mschart", 
	mschart_functions, // exported function table
	PHP_MINIT(mschart),
	NULL, 
	NULL, 
	NULL, 
	PHP_MINFO(mschart),
	PHP_MSCHART_VERSION,
	STANDARD_MODULE_PROPERTIES
};

#define REGISTER_MSCHART_CONSTANT(__c) REGISTER_LONG_CONSTANT(#__c, __c, CONST_CS | CONST_PERSISTENT)
#define MSCHART_FORMAT_JPEG 1
#define MSCHART_FORMAT_PNG  2
#define MSCHART_FORMAT_BMP  3
#define MSCHART_FORMAT_TIFF 4
#define MSCHART_FORMAT_MAX 4

PHP_MINIT_FUNCTION(mschart)
{
	REGISTER_MSCHART_CONSTANT(MSCHART_FORMAT_JPEG);
	REGISTER_MSCHART_CONSTANT(MSCHART_FORMAT_PNG);
	REGISTER_MSCHART_CONSTANT(MSCHART_FORMAT_BMP);
	REGISTER_MSCHART_CONSTANT(MSCHART_FORMAT_TIFF);

	return SUCCESS;
}

PHP_MINFO_FUNCTION(mschart)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "Microsoft Data Visualization Chart", "enabled");
	DISPLAY_INI_ENTRIES();
	php_info_print_table_end();
}

PHP_FUNCTION(mschart_loadxml)
{
	array<unsigned char> ^ buf = gcnew array<unsigned char>(4096);
	int b = 0;
	char* xml = NULL;
	int xml_len = 0;
	int image_format = 0;
	char *filename = NULL;
	int filename_len = 0;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|ls", &xml, &xml_len, &image_format, &filename, &filename_len) == FAILURE)
	{
			RETURN_FALSE;
	}

	if (image_format < 0 || image_format > MSCHART_FORMAT_MAX) {
		zend_throw_exception_ex(zend_exception_get_default(TSRMLS_C), 0 TSRMLS_CC, "Invalid image format");
	} else if (image_format == 0) {
		image_format = MSCHART_FORMAT_PNG;
	}

	String^ xmlString = gcnew String(xml, 0, xml_len);
	StringReader^ strReader = gcnew StringReader(xmlString);

	Chart ^chart_inst = gcnew Chart();
	try {
		chart_inst->Serializer->Load(strReader);

	/*TODO: Check if Serializer actually throws exception not extending the base Exeption class 
	 * Did not manage to throw such ex so far. Have asked the devs.
	 */
	} catch (Exception ^ex) {
		zend_throw_exception_ex(zend_exception_get_default(TSRMLS_C), 0 TSRMLS_CC, "Could not parse input: %s (%s)", ex->GetType()->FullName, ex->Message);
	}

	chart_inst->Serializer->Save("a1.xml");
	chart_inst->ChartAreas[0]->AxisX->Title = "Tilte axis";

	if (filename_len > 0) {
		String^ filenameString = gcnew String(filename, 0, filename_len);
		switch (image_format) {
			case MSCHART_FORMAT_JPEG:
					chart_inst->SaveImage(filenameString, ChartImageFormat::Jpeg);
				break;
			case MSCHART_FORMAT_BMP:
					chart_inst->SaveImage(filenameString, ChartImageFormat::Bmp);
				break;
			case MSCHART_FORMAT_TIFF:
					chart_inst->SaveImage(filenameString, ChartImageFormat::Tiff);
				break;
			default:
			case MSCHART_FORMAT_PNG:
					chart_inst->SaveImage(filenameString, ChartImageFormat::Png);
				break;
		}
	} else {
		MemoryStream ^chartStream = gcnew MemoryStream();
		switch (image_format) {
			case MSCHART_FORMAT_JPEG:
					chart_inst->SaveImage(chartStream, ChartImageFormat::Jpeg);
				break;
			case MSCHART_FORMAT_PNG:
					chart_inst->SaveImage(chartStream, ChartImageFormat::Png);
				break;
			case MSCHART_FORMAT_BMP:
					chart_inst->SaveImage(chartStream, ChartImageFormat::Bmp);
				break;
			case MSCHART_FORMAT_TIFF:
					chart_inst->SaveImage(chartStream, ChartImageFormat::Tiff);
				break;
		}

		//chart_inst->SaveImage(chartStream, ChartImageFormat::Jpeg);
		chartStream->Seek(0, SeekOrigin::Begin);
		pin_ptr<unsigned char> x = &buf[0];

		while ((b = chartStream->Read(buf, 0, buf->Length)) > 0) {
			php_write(x, b TSRMLS_CC);
		}
	}

	RETURN_TRUE;
}
