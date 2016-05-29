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
	class document;
	
	
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
		text()	{}
		explicit text( const std::string& inStr ) : actualText(inStr) {}
		
		virtual void	write( writer* inWriter, size_t depth );
		virtual void	print( size_t depth );

		std::string		actualText;
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
		tag() {}
		explicit tag( const std::string& inName ) : name(inName) {}
		
		virtual void	write( writer* inWriter, size_t depth );
		virtual void	print( size_t depth );
		
		virtual void		set_attribute( const std::string& inName, const std::string inValue );
		virtual std::string	get_attribute( const std::string& inName, const std::string& inDefault = "" );
		
		std::string				name;
		std::vector<attribute>	attributes;
	};
	
	
	class xml_reader
	{
	public:
		xml_reader( document& inDoc, const char* inString, size_t inLength );
		xml_reader( document& inDoc, FILE* inFile );
		
		document&							doc;
		std::vector<std::shared_ptr<node>>	nodes;
		std::string							currEntityName;
	};
	
	
	class writer
	{
	public:
		virtual ~writer()	{}
		
		virtual	void	write_node( std::shared_ptr<node> inNode, size_t depth );
		virtual	void	write_integer( long long inNum, size_t depth ) = 0;
		virtual	void	write_bool( bool inValue, size_t depth ) = 0;
		virtual	void	write_string( const std::string& inStr, size_t depth ) = 0;
		virtual void	write_open_tag_before_attributes( const std::string& inTagName, size_t numAttributes, size_t numChildren, size_t depth ) = 0;
		virtual void	write_open_tag_after_attributes( const std::string& inTagName, size_t numAttributes, size_t numChildren, size_t depth ) = 0;
		virtual void	write_attribute( const std::string& inName, const std::string& inValue ) = 0;
		virtual void	write_close_tag( const std::string& inTagName, size_t numChildren, size_t depth ) = 0;
	};
	
	
	class xml_writer : public writer
	{
	public:
		xml_writer( FILE* inFile ) : file(inFile), outStr(nullptr) {}
		xml_writer( std::string& outXmlString ) : file(nullptr), outStr(&outXmlString) {}
		
		virtual	void	write_integer( long long inNum, size_t depth ) override;
		virtual	void	write_bool( bool inValue, size_t depth ) override;
		virtual	void	write_string( const std::string& inStr, size_t depth ) override;
		virtual void	write_open_tag_before_attributes( const std::string& inTagName, size_t numAttributes, size_t numChildren, size_t depth ) override;
		virtual void	write_open_tag_after_attributes( const std::string& inTagName, size_t numAttributes, size_t numChildren, size_t depth ) override;
		virtual void	write_attribute( const std::string& inName, const std::string& inValue ) override;
		virtual void	write_close_tag( const std::string& inTagName, size_t numChildren, size_t depth ) override;
		
	protected:
		virtual void	output( const std::string& inStr );
		
		std::string	*outStr;
		FILE		*file;
	};
	
	
	class document
	{
	public:
		document();
		
		virtual void	write( writer* inWriter );
		
		void	add_xml_and_doctype_tags( const std::string& inType, const std::string& inDTD );
		
		std::shared_ptr<node>				root;
	};
	
} /* namespace dans_xml */

#endif /* dans_xml_hpp */
