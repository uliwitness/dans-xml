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


static string	path_next_to( string newFilename, string thePath )
{
	string::size_type slashPos = thePath.rfind( '/' );
	if( slashPos != string::npos )
		thePath = thePath.substr( 0, slashPos +1 );
	else
		thePath.erase();
	thePath.append( newFilename );
	
	return thePath;
}


static string	name_with_new_suffix( string thePath, string newSuffix )
{
	string::size_type slashPos = thePath.rfind( '/' );
	if( slashPos != string::npos )
		thePath = thePath.substr( slashPos +1 );
	string::size_type dotPos = thePath.rfind( '.' );
	if( dotPos != string::npos )
		thePath = thePath.substr( 0, dotPos );
	thePath.append( newSuffix );
	
	return thePath;
}


int main( int argc, const char * argv[] )
{
	int		result = 777;
	
	if( argc >= 2 && strcasecmp(argv[1],"test") == 0 )
	{
		printf( "=== Parse XML from a string & print it out:\n" );
		const char*		str = "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n<!DOCTYPE foo=bar thing silly=\"good thing\">\n<html>\n<head>\n<title>This &lt;may&gt; be neat</title>\n</head>\n<body bgcolor = \"#ffeeff\" border = 1></body></html>";
		document		theDoc;
		xml_reader	theReader( theDoc, str,strlen(str) );
		
		xml_writer	writer( stdout );
		theDoc.write( &writer );
		
		printf( "\n\n=== Build XML in memory and print it out:\n" );
		
		document	newDoc;
		newDoc.add_xml_and_doctype_tags( "html", "-//W3C.org//DTD xhtml V 2.0//EN", "");
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
		
		result = EXIT_SUCCESS;
	}
	else if( argc >= 4 && strcasecmp(argv[1],"print") == 0 )
	{
		document	theDoc;
		if( strcasecmp(argv[2],"--fromxml") == 0 )
		{
			const char*	srcPath = argv[3];
			FILE*		theFile = fopen(srcPath,"r");
			if( !theFile )
			{
				fprintf( stderr, "Can't find file \"%s\"\n", srcPath );
				return 1;
			}
			xml_reader	theReader( theDoc, theFile );
			fclose(theFile);
			
			result = EXIT_SUCCESS;
		}
		else if( strcasecmp(argv[2],"--fromdnxml") == 0 )
		{
			const char*	srcPath = argv[3];
			FILE*		theFile = fopen(srcPath,"r");
			if( !theFile )
			{
				fprintf( stderr, "Can't find file \"%s\"\n", srcPath );
				return 1;
			}
			binary_reader	theReader( theDoc, theFile );
			fclose(theFile);
			
			result = EXIT_SUCCESS;
		}
		
		if( result == EXIT_SUCCESS )
		{
			xml_writer	newWriter( stdout );
			theDoc.write( &newWriter );
		}
	}
	else if( argc >= 5 && strcasecmp(argv[1],"convert") == 0 )
	{
		document	theDoc;
		if( strcasecmp(argv[2],"--fromxml") == 0 )
		{
			FILE*		theFile = fopen(argv[4],"r");
			if( !theFile )
			{
				fprintf( stderr, "Can't find file \"%s\"\n", argv[3] );
				return 1;
			}
			xml_reader	theReader( theDoc, theFile );
			fclose(theFile);
			
			result = EXIT_SUCCESS;
		}
		else if( strcasecmp(argv[2],"--fromdnxml") == 0 )
		{
			FILE*			theFile = fopen(argv[4],"r");
			if( !theFile )
			{
				fprintf( stderr, "Can't find file \"%s\"\n", argv[3] );
				return 1;
			}
			binary_reader	theReader( theDoc, theFile );
			fclose(theFile);
			
			result = EXIT_SUCCESS;
		}
		else
		{
			fprintf( stderr, "Unknown import flag \"%s\", expected --fromxml or --fromdnxml here.\n", argv[2] );
			result = 3;
		}
		
		if( result == EXIT_SUCCESS && strcasecmp(argv[3],"--toxml") == 0 )
		{
			string	thePath;
			if( argc > 5 )
				thePath = argv[5];
			else
				thePath = path_next_to( name_with_new_suffix(argv[4],".xml"), argv[4] );
			FILE*	outputFile = fopen( thePath.c_str(), "w" );
			if( !outputFile )
			{
				fprintf( stderr, "Can't create file \"%s\"\n", thePath.c_str() );
				return 2;
			}
			xml_writer	newWriter( outputFile );
			theDoc.write( &newWriter );
			
			result = EXIT_SUCCESS;
		}
		else if( result == EXIT_SUCCESS && strcasecmp(argv[3],"--todnxml") == 0 )
		{
			string	thePath;
			if( argc > 5 )
				thePath = argv[5];
			else
				thePath = path_next_to( name_with_new_suffix(argv[4],".dnxml"), argv[4] );
			FILE*	outputFile = fopen( thePath.c_str(), "w" );
			if( !outputFile )
			{
				fprintf( stderr, "Can't create file \"%s\"\n", thePath.c_str() );
				return 2;
			}
			binary_writer	newWriter( outputFile );
			theDoc.write( &newWriter );
			
			result = EXIT_SUCCESS;
		}
		else
		{
			fprintf( stderr, "Unknown export flag \"%s\", expected --toxml or --todnxml here.\n", argv[2] );
			result = 4;
		}
	}
	
	if( result != EXIT_SUCCESS )
	{
		fprintf(stderr,"Syntax: %s <mode> ...\n\ttest\n\tprint --fromxml|--fromdnxml <fromFilename>\n\tconvert --fromxml|--fromdnxml --toxml|--todnxml <srcFilename> [<dstFilename>]\n", argv[0]);
	}

    return result;
}
