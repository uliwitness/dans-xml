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


int main( int argc, const char * argv[] )
{
	const char*		str = "<!DOCTYPE foo=bar thing silly=\"good thing\">\n<html>\n<head>\n<title>This &lt;may&gt; be neat</title>\n</head>\n<body bgcolor = \"#ffeeff\" border = 1></body></html>";
	dans_xml::document		theDoc(str,strlen(str));
	
    return 0;
}
