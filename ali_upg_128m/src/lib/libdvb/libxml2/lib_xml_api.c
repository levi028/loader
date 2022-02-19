/*****************************************************************************
*    Copyright (C) 2009 ALi Corp. All Rights Reserved.
*
*    Company confidential and Properietary information.
*    This information may not be disclosed to unauthorized
*    individual.
*    File: lib_xml_api.c
*
*    Description:
*
*    History:
*    Date           Athor        Version        Reason
*    ========       ========     ========       ========
*
*
*****************************************************************************/
#include <api/libxml2/parser.h>
#include <api/libxml2/tree.h>
#include <api/libxml2/xpath.h>
#include <api/libxml2/xpathInternals.h>

/**
 * register_namespaces_ex:
 * @xpathCtx:        the pointer to an XPath context.
 * @nsList:     the list of known namespaces in
 *            "<prefix1>=<href1> <prefix2>=href2> ..." format.
 *
 * Registers namespaces from @nsList in @xpathCtx.
 *
 * Returns 0 on success and a negative value otherwise.
 */
int
register_namespaces_ex(xmlXPathContextPtr xpathCtx, const xmlChar* nsList) {
    xmlChar* nsListDup;
    xmlChar* prefix;
    xmlChar* href;
    xmlChar* next;

    //assert(xpathCtx);
   // assert(nsList);

    nsListDup = xmlStrdup(nsList);
    if(nsListDup == NULL) {
	//libc_printf("Error: unable to strdup namespaces list\n");
    //libc_printf(stderr, "Error: unable to strdup namespaces list\n");
    return(-1);
    }

    next = nsListDup;
    while(next != NULL) {
    /* skip spaces */
    while((*next) == ' ') next++;
    if((*next) == '\0') break;

    /* find prefix */
    prefix = next;
    next = (xmlChar*)xmlStrchr(next, '=');
    if(next == NULL) {
		//libc_printf("Error: invalid namespaces list format\n");
        //libc_printf(stderr,"Error: invalid namespaces list format\n");
        xmlFree(nsListDup);
        return(-1);
    }
    *(next++) = '\0';

    /* find href */
    href = next;
    next = (xmlChar*)xmlStrchr(next, ' ');
    if(next != NULL) {
        *(next++) = '\0';
    }

    /* do register namespace */
    if(xmlXPathRegisterNs(xpathCtx, prefix, href) != 0) {
		//libc_printf("Error: unable to register NS with prefix=\"%s\" and href=\"%s\"\n", prefix, href);
        //libc_printf(stderr,"Error: unable to register NS with prefix=\"%s\" and href=\"%s\"\n", prefix, href);
        xmlFree(nsListDup);
        return(-1);
    }
    }

    xmlFree(nsListDup);
    return(0);
}


xmlXPathObjectPtr
getnodeset_ex (xmlDocPtr doc, xmlChar *xpath, xmlChar* nsList){

    xmlXPathContextPtr context;
    xmlXPathObjectPtr result;
    //xmlChar *nsList ="yweather=http://xml.weather.yahoo.com/ns/rss/1.0 geo=http://www.w3.org/2003/01/geo/wgs84_pos#";


    context = xmlXPathNewContext(doc);
    if (context == NULL) {
		//libc_printf("Error in xmlXPathNewContext\n");
        return NULL;
    }
#if 1
    /* Register namespaces from list (if any) */
    if((nsList != NULL) && (register_namespaces_ex(context, nsList) < 0)) {
		//libc_printf("Error: failed to register namespaces list\n");
        //fprintf(stderr,"Error: failed to register namespaces list \"%s\"\n", nsList);
        xmlXPathFreeContext(context);
        xmlFreeDoc(doc);
        return NULL;
        }
#endif
    result = xmlXPathEvalExpression(xpath, context);
    xmlXPathFreeContext(context);
    if (result == NULL) {
		//libc_printf("Error in xmlXPathEvalExpression\n");
        return NULL;
    }
    if(xmlXPathNodeSetIsEmpty(result->nodesetval)){
        xmlXPathFreeObject(result);
				//libc_printf("No result\n");
        return NULL;
    }
    return result;
}

