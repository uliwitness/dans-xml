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
	class writer;
	
	class node
	{
	public:
		virtual ~node()	{}
		
		virtual void	write( writer* inWriter, size_t depth );
		virtual void	print( size_t depth );
		
		std::weak_ptr<node>					parent;
		std::vector<std::shared_ptr<node>>	children;
	};
	
	
	class text : public node
	{
	public:
		virtual void	write( writer* inWriter, size_t depth );
		virtual void	print( size_t depth );

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
		virtual void	write( writer* inWriter, size_t depth );
		virtual void	print( size_t depth );
		
		std::string				name;
		std::vector<attribute>	attributes;
	};
	
	
	class writer
	{
	public:
		virtual ~writer()	{}
		
		virtual	void	write_node( std::shared_ptr<node> inNode, size_t depth );
		virtual	void	write_integer( long long inNum, size_t depth ) = 0;
		virtual	void	write_bool( bool inValue, size_t depth ) = 0;
		virtual	void	write_string( const std::string& inStr, size_t depth ) = 0;
		virtual void	write_open_tag_before_attributes( const std::string& inTagName, size_t numChildren, size_t depth ) = 0;
		virtual void	write_open_tag_after_attributes( const std::string& inTagName, size_t numChildren, size_t depth ) = 0;
		virtual void	write_attribute( const std::string& inName, const std::string& inValue ) = 0;
		virtual void	write_close_tag( const std::string& inTagName, size_t numChildren, size_t depth ) = 0;
	};
	
	
	class xml_writer : public writer
	{
	public:
		xml_writer( FILE* inFile ) : file(inFile), outStr(nullptr) {}
		xml_writer( std::string& outXmlString ) : file(nullptr), outStr(&outXmlString) {}
		
		virtual	void	write_integer( long long inNum, size_t depth );
		virtual	void	write_bool( bool inValue, size_t depth );
		virtual	void	write_string( const std::string& inStr, size_t depth );
		virtual void	write_open_tag_before_attributes( const std::string& inTagName, size_t numChildren, size_t depth );
		virtual void	write_open_tag_after_attributes( const std::string& inTagName, size_t numChildren, size_t depth );
		virtual void	write_attribute( const std::string& inName, const std::string& inValue );
		virtual void	write_close_tag( const std::string& inTagName, size_t numChildren, size_t depth );
		
	protected:
		virtual void	output( const std::string& inStr );
		
		std::string	*outStr;
		FILE		*file;
	};
	
	
	class document
	{
	public:
		document( const char* inString, size_t inLength );
		document( FILE* inFile );
		
		virtual void	write( writer* inWriter );
		
		std::vector<std::shared_ptr<node>>	nodes;
		std::shared_ptr<node>				root;
		std::string							currEntityName;
	};
	
} /* namespace dans_xml */

#endif /* dans_xml_hpp */
