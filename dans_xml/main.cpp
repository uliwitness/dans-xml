//
//  main.cpp
//  dans_xml
//
//  Created by Uli Kusterer on 28/05/16.
//  Copyright © 2016 Uli Kusterer. All rights reserved.
//

#include <iostream>
#include "dans_xml.hpp"

using namespace dans_xml;
using namespace std;


int main( int argc, const char * argv[] )
{
	const char*		str = "<!DOCTYPE foo=bar thing silly=\"good thing\">\n<html>\n<head>\n<title>This &lt;may&gt; be neat</title>\n</head>\n<body bgcolor = \"#ffeeff\" border = 1></body></html>";
	document		theDoc;
	xml_reader	theReader( theDoc, str,strlen(str) );
	
	xml_writer	writer( stdout );
	theDoc.write( &writer );
	
	printf( "\n\n" );
	
	document	newDoc;
	shared_ptr<tag>	doctype = make_shared<tag>("!DOCTYPE");
	doctype->set_attribute( "foo", "bar" );
	doctype->set_attribute( "thing", "" );
	doctype->set_attribute( "silly", "good thing" );
	newDoc.root->children.push_back( doctype );
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

	printf( "\n" );

    return 0;
}
