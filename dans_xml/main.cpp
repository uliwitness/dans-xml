//
//  main.cpp
//  dans_xml
//
//  Created by Uli Kusterer on 28/05/16.
//  Copyright © 2016 Uli Kusterer. All rights reserved.
//

#include <iostream>
#include "dans_xml.hpp"
#include "dans_binary.hpp"

using namespace dans_xml;
using namespace std;


string	path_next_to( string newFilename, string thePath )
{
	string::size_type slashPos = thePath.rfind( '/' );
	if( slashPos != string::npos )
		thePath = thePath.substr( 0, slashPos +1 );
	else
		thePath.erase();
	thePath.append( newFilename );
	
	return thePath;
}


int main( int argc, const char * argv[] )
{
	printf( "=== Parse XML from a string & print it out:\n" );
	const char*		str = "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n<!DOCTYPE foo=bar thing silly=\"good thing\">\n<html>\n<head>\n<title>This &lt;may&gt; be neat</title>\n</head>\n<body bgcolor = \"#ffeeff\" border = 1></body></html>";
	document		theDoc;
	xml_reader	theReader( theDoc, str,strlen(str) );
	
	xml_writer	writer( stdout );
	theDoc.write( &writer );
	
	printf( "\n\n=== Build XML in memory and print it out:\n" );
	
	document	newDoc;
	newDoc.add_xml_and_doctype_tags( "html", "-//W3C.org//DTD xhtml V 2.0//EN");
	shared_ptr<tag>	html = make_shared<tag>("html");
	newDoc.root->children.push_back( html );
	shared_ptr<tag>	head = make_shared<tag>("head");
	html->children.push_back( head );
	shared_ptr<tag>	title = make_shared<tag>("title");
	head->children.push_back( title );
	shared_ptr<text>	titleText = make_shared<text>("This <may> be neat");
	title->children.push_back( titleText );
	shared_ptr<tag>	body = make_shared<tag>("body");
	body->set_attribute( "bgcolor", "#ffeeff" );
	body->set_attribute( "border", "1" );
	html->children.push_back( body );
	xml_writer	newWriter( stdout );
	newDoc.write( &newWriter );

	string		thePath = path_next_to( "newDoc.xml", argv[0] );
	printf( "\n\n=== Build XML in memory and write it to file: %s\n", thePath.c_str() );
	FILE*		theXMLFile = fopen( thePath.c_str(), "w" );
	xml_writer	newWriter2( theXMLFile );
	newDoc.write( &newWriter2 );
	fclose( theXMLFile );

	thePath = path_next_to( "newDoc.dnxml", argv[0] );
	printf( "\n\n=== Write that XML out as Dan's Binary to file %s:\n", thePath.c_str() );
	FILE*	theBinaryFile = fopen( thePath.c_str(), "w" );
	binary_writer	binaryWriter( theBinaryFile );
	theDoc.write( &binaryWriter );
	fclose( theBinaryFile );
	
	printf( "\n\n=== Read Dan's Binary back in and print it out:\n" );
	FILE*		theBinaryFile2 = fopen( thePath.c_str(), "r" );
	document	binDoc;
	binary_reader	binaryReader( binDoc, theBinaryFile2 );
	fclose( theBinaryFile2 );
	
	xml_writer	newWriter3( stdout );
	binDoc.write( &newWriter3 );

	printf( "\n" );

    return 0;
}
