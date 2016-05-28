//
//  dans_xml.cpp
//  dans_xml
//
//  Created by Uli Kusterer on 28/05/16.
//  Copyright © 2016 Uli Kusterer. All rights reserved.
//

#include "dans_xml.hpp"
#include <iostream>
#include <map>


using namespace std;
using namespace dans_xml;


typedef void (*func_ptr)();

typedef func_ptr (*eat_char_fcn)( char currCh, xml_reader& reader, vector<shared_ptr<node>>& nod, attribute* att );	// Returns another function of its type.


func_ptr	eat_whitespace( char currCh, xml_reader& reader, vector<shared_ptr<node>>& nod, attribute* att );
func_ptr	eat_tag_name( char currCh, xml_reader& reader, vector<shared_ptr<node>>& nod, attribute* att );
func_ptr	eat_tag_attr_name( char currCh, xml_reader& reader, vector<shared_ptr<node>>& nod, attribute* att );
func_ptr	eat_tag_attr_value( char currCh, xml_reader& reader, vector<shared_ptr<node>>& nod, attribute* att );
func_ptr	eat_tag_attr_value_quoted( char currCh, xml_reader& reader, vector<shared_ptr<node>>& nod, attribute* att );
func_ptr	eat_tag_attr_value_unquoted( char currCh, xml_reader& reader, vector<shared_ptr<node>>& nod, attribute* att );
func_ptr	eat_text_chars( char currCh, xml_reader& reader, vector<shared_ptr<node>>& nod, attribute* att );
func_ptr	eat_entity_chars( char currCh, xml_reader& reader, vector<shared_ptr<node>>& nod, attribute* att );


func_ptr	eat_text_chars( char currCh, xml_reader& reader, vector<shared_ptr<node>>& nod, attribute* att )
{
	switch( currCh )
	{
		case '<':
		{
			shared_ptr<text> textNode = dynamic_pointer_cast<text,node>(nod.back());
			if( textNode )
			{
				nod.pop_back();
				nod.back()->children.push_back(textNode);
			}
			shared_ptr<node>	theNode = make_shared<tag>();
			nod.push_back( theNode );
			return (func_ptr)eat_tag_name;
			break;
		}
		
		case '&':
			return (func_ptr)eat_entity_chars;
			break;
		
		default:
		{
			shared_ptr<text> theNode = dynamic_pointer_cast<text,node>(nod.back());
			if( !theNode )
			{
				theNode = make_shared<text>();
				nod.push_back( theNode );
			}
			theNode->text.append(1,currCh);
			return (func_ptr)eat_text_chars;
			break;
		}
	}
	
	return nullptr;
}


func_ptr	eat_entity_chars( char currCh, xml_reader& reader, vector<shared_ptr<node>>& nod, attribute* att )
{
	static std::map<std::string,std::string>	sEntities;
	if( sEntities.size() == 0 )
	{
		sEntities["lt"] = "<";
		sEntities["gt"] = ">";
		sEntities["amp"] = "&";
	}
	
	switch( currCh )
	{
		case ';':
		{
			shared_ptr<text> theNode = dynamic_pointer_cast<text,node>(nod.back());
			theNode->text.append(sEntities[reader.currEntityName]);
			reader.currEntityName.erase();
			return (func_ptr)eat_text_chars;
			break;
		}
		
		default:
		{
			reader.currEntityName.append(1,currCh);
			return (func_ptr)eat_entity_chars;
			break;
		}
	}
	
	return nullptr;
}


func_ptr	eat_tag_attr_value_quoted_entity( char currCh, xml_reader& reader, vector<shared_ptr<node>>& nod, attribute* att )
{
	static std::map<std::string,std::string>	sEntities;
	if( sEntities.size() == 0 )
	{
		sEntities["lt"] = "<";
		sEntities["gt"] = ">";
		sEntities["amp"] = "&";
	}
	
	switch( currCh )
	{
		case ';':
		{
			shared_ptr<text> theNode = dynamic_pointer_cast<text,node>(nod.back());
			theNode->text.append(sEntities[reader.currEntityName]);
			reader.currEntityName.erase();
			return (func_ptr)eat_text_chars;
			break;
		}
		
		default:
		{
			reader.currEntityName.append(1,currCh);
			return (func_ptr)eat_tag_attr_value_quoted;
			break;
		}
	}
	
	return nullptr;
}


func_ptr	eat_whitespace( char currCh, xml_reader& reader, vector<shared_ptr<node>>& nod, attribute* att )
{
	switch( currCh )
	{
		case ' ':
		case '\t':
		case '\r':
		case '\n':
			// Just skip.
			return (func_ptr)eat_whitespace;
			break;
		
		case '<':
		{
			shared_ptr<node>	theNode = make_shared<tag>();
			nod.push_back( theNode );
			return (func_ptr)eat_tag_name;
			break;
		}
		
		default:
			return eat_text_chars( currCh, reader, nod, att );
			break;
	}
	
	return nullptr;
}


func_ptr	eat_tag_name( char currCh, xml_reader& reader, vector<shared_ptr<node>>& nod, attribute* att )
{
	switch( currCh )
	{
		case ' ':
		case '\t':
		case '\r':
		case '\n':
			return (func_ptr)eat_tag_attr_name;
			break;
		
		case '>':
		{
			shared_ptr<tag> prevNode = static_pointer_cast<tag,node>(nod.back());
			if( prevNode->name.length() > 0 && prevNode->name[0] == '/' )
			{
				nod.pop_back();
				shared_ptr<tag> closedNode = static_pointer_cast<tag,node>(nod.back());
				nod.pop_back();
				nod.back()->children.push_back( closedNode );
			}
			else if( (prevNode->name.length() > 0 && prevNode->name[prevNode->name.length() -1] == '/')
					|| (prevNode->name.length() > 0 && prevNode->name[0] == '!')
					|| (prevNode->name.length() > 0 && prevNode->name[0] == '?') )
			{
				nod.pop_back();
				nod.back()->children.push_back( prevNode );
			}
			return (func_ptr)eat_whitespace;
			break;
		}
		
		default:
		{
			shared_ptr<tag>	currTag = static_pointer_cast<tag,node>(nod.back());
			currTag->name.append(1, currCh);
			break;
		}
	}
	
	return (func_ptr)eat_tag_name;
}


func_ptr	eat_tag_attr_name_whitespace( char currCh, xml_reader& reader, vector<shared_ptr<node>>& nod, attribute* att )
{
	switch( currCh )
	{
		case ' ':
		case '\t':
		case '\r':
		case '\n':
			return (func_ptr)eat_tag_attr_name_whitespace;
			break;
		
		case '=':
			return (func_ptr)eat_tag_attr_value;
			break;
		
		case '>':
		{
			shared_ptr<tag> prevNode = static_pointer_cast<tag,node>(nod.back());
			if( att->name.size() > 0 )
				prevNode->attributes.push_back(*att);
			att->name.erase();
			att->value.erase();
			if( prevNode->name.length() > 0 && prevNode->name[0] == '/' )
			{
				nod.pop_back();
				shared_ptr<tag> closedNode = static_pointer_cast<tag,node>(nod.back());
				nod.pop_back();
				nod.back()->children.push_back( closedNode );
			}
			else if( (prevNode->name.length() > 0 && prevNode->name[prevNode->name.length() -1] == '/')
					|| (prevNode->name.length() > 0 && prevNode->name[0] == '!')
					|| (prevNode->name.length() > 0 && prevNode->name[0] == '?') )
			{
				nod.pop_back();
				nod.back()->children.push_back( prevNode );
			}
			return (func_ptr)eat_whitespace;
			break;
		}
		
		default:
		{
			shared_ptr<tag> prevNode = static_pointer_cast<tag,node>(nod.back());
			if( att->name.size() > 0 )
				prevNode->attributes.push_back(*att);
			att->name.erase();
			att->value.erase();
			att->name.append(1, currCh);
			return (func_ptr)eat_tag_attr_name;
			break;
		}
	}
	
	return nullptr;
}


func_ptr	eat_tag_attr_name( char currCh, xml_reader& reader, vector<shared_ptr<node>>& nod, attribute* att )
{
	switch( currCh )
	{
		case ' ':
		case '\t':
		case '\r':
		case '\n':
			return (func_ptr)eat_tag_attr_name_whitespace;
			break;
		
		case '=':
			return (func_ptr)eat_tag_attr_value;
			break;
		
		case '>':
		{
			shared_ptr<tag> prevNode = static_pointer_cast<tag,node>(nod.back());
			if( att->name.size() > 0 )
				prevNode->attributes.push_back(*att);
			att->name.erase();
			att->value.erase();
			if( prevNode->name.length() > 0 && prevNode->name[0] == '/' )
			{
				nod.pop_back();
				shared_ptr<tag> closedNode = static_pointer_cast<tag,node>(nod.back());
				nod.pop_back();
				nod.back()->children.push_back( closedNode );
			}
			else if( (prevNode->name.length() > 0 && prevNode->name[prevNode->name.length() -1] == '/')
					|| (prevNode->name.length() > 0 && prevNode->name[0] == '!')
					|| (prevNode->name.length() > 0 && prevNode->name[0] == '?') )
			{
				nod.pop_back();
				nod.back()->children.push_back( prevNode );
			}
			return (func_ptr)eat_whitespace;
			break;
		}
		
		default:
			att->name.append(1, currCh);
			return (func_ptr)eat_tag_attr_name;
			break;
	}
	
	return nullptr;
}


func_ptr	eat_tag_attr_value( char currCh, xml_reader& reader, vector<shared_ptr<node>>& nod, attribute* att )
{
	switch( currCh )
	{
		case ' ':
		case '\t':
		case '\r':
		case '\n':
			return (func_ptr)eat_tag_attr_value;
			break;
		
		case '"':
			return (func_ptr)eat_tag_attr_value_quoted;
			break;
		
		case '>':
			return nullptr;
			break;
		
		default:
			att->value.append(1,currCh);
			return (func_ptr)eat_tag_attr_value_unquoted;
			break;
	}
	
	return nullptr;
}


func_ptr	eat_tag_attr_value_quoted( char currCh, xml_reader& reader, vector<shared_ptr<node>>& nod, attribute* att )
{
	switch( currCh )
	{
		case '"':
		{
			shared_ptr<tag> prevNode = static_pointer_cast<tag,node>(nod.back());
			if( att->name.size() > 0 )
				prevNode->attributes.push_back(*att);
			att->name.erase();
			att->value.erase();
			return (func_ptr)eat_tag_attr_name;
			break;
		}
		
		case '&':
			return (func_ptr)eat_tag_attr_value_quoted_entity;
			break;
		
		default:
			att->value.append( 1, currCh );
			return (func_ptr)eat_tag_attr_value_quoted;
			break;
	}
	
	return nullptr;
}


func_ptr	eat_tag_attr_value_unquoted( char currCh, xml_reader& reader, vector<shared_ptr<node>>& nod, attribute* att )
{
	switch( currCh )
	{
		case ' ':
		case '\t':
		case '\r':
		case '\n':
		{
			shared_ptr<tag>	currTag = static_pointer_cast<tag,node>(nod.back());
			if( att->name.size() > 0 )
				currTag->attributes.push_back(*att);
			att->name.erase();
			att->value.erase();

			return (func_ptr)eat_tag_attr_name;
			break;
		}
		
		case '"':
			return nullptr;
			break;
		
		case '>':
		{
			shared_ptr<tag> prevNode = static_pointer_cast<tag,node>(nod.back());
			if( att->name.size() > 0 )
				prevNode->attributes.push_back(*att);
			att->name.erase();
			att->value.erase();
			if( prevNode->name.length() > 0 && prevNode->name[0] == '/' )
			{
				nod.pop_back();
				shared_ptr<tag> closedNode = static_pointer_cast<tag,node>(nod.back());
				nod.pop_back();
				nod.back()->children.push_back( closedNode );
			}
			else if( (prevNode->name.length() > 0 && prevNode->name[prevNode->name.length() -1] == '/')
					|| (prevNode->name.length() > 0 && prevNode->name[0] == '!')
					|| (prevNode->name.length() > 0 && prevNode->name[0] == '?') )
			{
				nod.pop_back();
				nod.back()->children.push_back( prevNode );
			}
			return (func_ptr)eat_whitespace;
			break;
		}
		
		default:
			att->value.append( 1, currCh );
			return (func_ptr)eat_tag_attr_value_unquoted;
			break;
	}
	
	return nullptr;
}


xml_reader::xml_reader( document& inDoc, const char* inString, size_t inLength )
	: doc(inDoc)
{
	vector<shared_ptr<node>>	nod;
	inDoc.root = shared_ptr<node>(new node);
	nod.push_back( inDoc.root );
	attribute					att;
	eat_char_fcn				state = eat_whitespace;
	size_t						x = 0;
	while( (x < inLength) && state )
	{
		state = (eat_char_fcn)state( inString[x++], *this, nod, &att );
	}
}


xml_reader::xml_reader( document& inDoc, FILE* inFile )
: doc(inDoc)
{
	vector<shared_ptr<node>>	nod;
	inDoc.root = shared_ptr<node>(new node);
	nod.push_back( inDoc.root );
	attribute					att;
	eat_char_fcn				state = eat_whitespace;
	while( feof(inFile) != 0 && state )
	{
		state = (eat_char_fcn)state( fgetc(inFile), *this, nod, &att );
	}
}


document::document()
{
	root = shared_ptr<node>(new node);
}


void	document::write( writer* inWriter )
{
	inWriter->write_node( root, 0 );
}


void	writer::write_node( std::shared_ptr<node> inNode, size_t depth )
{
	inNode->write( this, depth );
}


void	xml_writer::write_integer( long long inNum, size_t depth )
{
	output( std::to_string( inNum ) );
}


void	xml_writer::write_bool( bool inValue, size_t depth )
{
	output( inValue ? "<true />" : "<false />" );
}


void	xml_writer::write_string( const std::string& inStr, size_t depth )
{
	output( inStr );
}


void	xml_writer::write_open_tag_before_attributes( const std::string& inTagName, size_t numChildren, size_t depth )
{
	output( "<" );
	output( inTagName );
}


void	xml_writer::write_open_tag_after_attributes( const std::string& inTagName, size_t numChildren, size_t depth )
{
	if( numChildren == 0 && inTagName.size() != 0 && inTagName[0] != '!' && inTagName[0] != '?' )
		output( " />" );
	else
		output( ">" );
}


void	xml_writer::write_attribute( const std::string& inName, const std::string& inValue )
{
	output( " " );
	output( inName );
	output( "=\"" );
	output( inValue );
	output( "\"" );
}


void	xml_writer::write_close_tag( const std::string& inTagName, size_t numChildren, size_t depth )
{
	if( numChildren > 0 && (inTagName.size() == 0 || inTagName[0] != '!' || inTagName[0] != '?') )
	{
		output( "</" );
		output( inTagName );
		output( ">" );
	}
}


void	xml_writer::output( const std::string& inStr )
{
	if( file )
		fwrite( inStr.data(), 1, inStr.size(), file );
	else
		outStr->append( inStr );
}


void	node::write( writer* inWriter, size_t depth )
{
	for( auto child : children )
		child->write( inWriter, depth );
}


void	node::print( size_t depth )
{
	for( auto child : children )
		child->print( depth );
}


void	tag::write( writer* inWriter, size_t depth )
{
	size_t numChildren = children.size();
	inWriter->write_open_tag_before_attributes( name, numChildren, depth );
	for( const attribute& att : attributes )
	{
		inWriter->write_attribute( att.name, att.value );
	}
	
	inWriter->write_open_tag_after_attributes( name, numChildren, depth );
	
	node::write( inWriter, depth +1 );
		
	inWriter->write_close_tag( name, numChildren, depth );
}


void	tag::print( size_t depth )
{
	cout << "<" << name;
	for( const attribute& att : attributes )
	{
		cout << " " << att.name << "=\"" << att.value << "\"";
	}
	
	if( name.length() == 0 || name[0] == '!' || name[0] == '?' )
		cout << ">";	// That's it, just print that tag.
	else if( children.size() == 0 )
		cout << " />";
	else
	{
		cout << ">";
		
		node::print( depth +1 );
		
		cout << "</" << name << ">";
	}
}


void	text::write( writer* inWriter, size_t depth )
{
	std::string	currText;
	size_t	len = text.length();
	for( size_t x = 0; x < len; x++ )
	{
		char	currCh = text[x];
		switch( currCh )
		{
			case '<':
				currText.append( "&lt;" );
				break;
				
			case '>':
				currText.append( "&gt;" );
				break;
				
			case '&':
				currText.append( "&amp;" );
				break;
				
			default:
				currText.append( 1, currCh );
		}
	}
	
	inWriter->write_string( currText, depth );
}


void	text::print( size_t depth )
{
	size_t	len = text.length();
	for( size_t x = 0; x < len; x++ )
	{
		char	currCh = text[x];
		switch( currCh )
		{
			case '<':
				cout << "&lt;";
				break;
			
			case '>':
				cout << "&gt;";
				break;
			
			case '&':
				cout << "&amp;";
				break;
			
			default:
				cout << currCh;
		}
	}
}

