//
//  dans_xml.cpp
//  dans_xml
//
//  Created by Uli Kusterer on 28/05/16.
//  Copyright © 2016 Uli Kusterer. All rights reserved.
//

#include "dans_xml.hpp"
#include <iostream>


using namespace std;
using namespace dans_xml;


typedef void (*func_ptr)();

typedef func_ptr (*eat_char_fcn)( char currCh, document* doc, shared_ptr<node>* nod, attribute* att );	// Returns another function of its type.


func_ptr	eat_whitespace( char currCh, document* doc, shared_ptr<node>* nod, attribute* att );
func_ptr	eat_tag_name( char currCh, document* doc, shared_ptr<node>* nod, attribute* att );
func_ptr	eat_tag_attr_name( char currCh, document* doc, shared_ptr<node>* nod, attribute* att );
func_ptr	eat_tag_attr_value( char currCh, document* doc, shared_ptr<node>* nod, attribute* att );
func_ptr	eat_tag_attr_value_quoted( char currCh, document* doc, shared_ptr<node>* nod, attribute* att );
func_ptr	eat_tag_attr_value_unquoted( char currCh, document* doc, shared_ptr<node>* nod, attribute* att );



func_ptr	eat_whitespace( char currCh, document* doc, shared_ptr<node>* nod, attribute* att )
{
	switch( currCh )
	{
		case ' ':
		case '\t':
		case '\r':
		case '\n':
			// Just skip.
			break;
		
		case '<':
			(*nod) = make_shared<tag>();
			return (func_ptr)eat_tag_name;
			break;
	}
	
	return (func_ptr)eat_whitespace;
}


func_ptr	eat_tag_name( char currCh, document* doc, shared_ptr<node>* nod, attribute* att )
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
			doc->nodes.push_back( *nod );
			*nod = nullptr;
			return (func_ptr)eat_whitespace;
			break;
		
		default:
			shared_ptr<tag>	currTag = static_pointer_cast<tag,node>(*nod);
			currTag->name.append(1, currCh);
			break;
	}
	
	return (func_ptr)eat_tag_name;
}


func_ptr	eat_tag_attr_name_whitespace( char currCh, document* doc, shared_ptr<node>* nod, attribute* att )
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
			shared_ptr<tag>	currTag = static_pointer_cast<tag,node>(*nod);
			if( att->name.size() > 0 )
				currTag->attributes.push_back(*att);
			att->name.erase();
			att->value.erase();
			doc->nodes.push_back( *nod );
			*nod = nullptr;
			return (func_ptr)eat_whitespace;
			break;
		}
		
		default:
		{
			shared_ptr<tag>	currTag = static_pointer_cast<tag,node>(*nod);
			if( att->name.size() > 0 )
				currTag->attributes.push_back(*att);
			att->name.erase();
			att->value.erase();
			att->name.append(1, currCh);
			return (func_ptr)eat_tag_attr_name;
			break;
		}
	}
	
	return nullptr;
}


func_ptr	eat_tag_attr_name( char currCh, document* doc, shared_ptr<node>* nod, attribute* att )
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
			shared_ptr<tag>	currTag = static_pointer_cast<tag,node>(*nod);
			if( att->name.size() > 0 )
				currTag->attributes.push_back(*att);
			att->name.erase();
			att->value.erase();
			doc->nodes.push_back( *nod );
			*nod = nullptr;
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


func_ptr	eat_tag_attr_value( char currCh, document* doc, shared_ptr<node>* nod, attribute* att )
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


func_ptr	eat_tag_attr_value_quoted( char currCh, document* doc, shared_ptr<node>* nod, attribute* att )
{
	switch( currCh )
	{
		case '"':
		{
			shared_ptr<tag>	currTag = static_pointer_cast<tag,node>(*nod);
			if( att->name.size() > 0 )
				currTag->attributes.push_back(*att);
			att->name.erase();
			att->value.erase();
			return (func_ptr)eat_tag_attr_name;
			break;
		}
		
		default:
			att->value.append( 1, currCh );
			return (func_ptr)eat_tag_attr_value_quoted;
			break;
	}
	
	return nullptr;
}


func_ptr	eat_tag_attr_value_unquoted( char currCh, document* doc, shared_ptr<node>* nod, attribute* att )
{
	switch( currCh )
	{
		case ' ':
		case '\t':
		case '\r':
		case '\n':
		{
			shared_ptr<tag>	currTag = static_pointer_cast<tag,node>(*nod);
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
			shared_ptr<tag>	currTag = static_pointer_cast<tag,node>(*nod);
			if( att->name.size() > 0 )
				currTag->attributes.push_back(*att);
			att->name.erase();
			att->value.erase();
			doc->nodes.push_back( *nod );
			*nod = nullptr;
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


document::document( const char* inString, size_t inLength )
{
	shared_ptr<node>	nod;
	attribute			att;
	eat_char_fcn		state = eat_whitespace;
	size_t				x = 0;
	while( (x < inLength) && state )
	{
		state = (eat_char_fcn)state( inString[x++], this, &nod, &att );
	}
	
	for( auto node : nodes )
	{
		node->print();
	}
}


void	tag::print()
{
	cout << "[<" << name;
	for( const attribute& att : attributes )
	{
		cout << " [" << att.name << "]=\"" << att.value << "\"";
	}
	cout << ">]";
}