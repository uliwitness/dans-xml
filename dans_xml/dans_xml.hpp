//
//  dans_xml.hpp
//  dans_xml
//
//  Created by Uli Kusterer on 28/05/16.
//  Copyright © 2016 Uli Kusterer. All rights reserved.
//

#ifndef dans_xml_hpp
#define dans_xml_hpp

#include <vector>
#include <memory>
#include <string>


namespace dans_xml
{
	
	class node
	{
	public:
		virtual ~node()	{}
		
		virtual void	print();
		
		std::weak_ptr<node>					parent;
		std::vector<std::shared_ptr<node>>	children;
	};
	
	
	class text : public node
	{
	public:
		virtual void	print();

		std::string		text;
	};
	
	
	class attribute
	{
	public:
		std::string		name;
		std::string		value;
	};
	
	
	class tag : public node
	{
	public:
		virtual void	print();
		
		std::string				name;
		std::vector<attribute>	attributes;
	};
	
	class document
	{
	public:
		document( const char* inString, size_t inLength );
		
		std::vector<std::shared_ptr<node>>	nodes;
		std::string							currEntityName;
	};
	
} /* namespace dans_xml */

#endif /* dans_xml_hpp */
