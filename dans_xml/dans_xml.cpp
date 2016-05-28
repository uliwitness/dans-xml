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

typedef func_ptr (*eat_char_fcn)( char currCh, document* doc, vector<shared_ptr<node>>& nod, attribute* att );	// Returns another function of its type.


func_ptr	eat_whitespace( char currCh, document* doc, vector<shared_ptr<node>>& nod, attribute* att );
func_ptr	eat_tag_name( char currCh, document* doc, vector<shared_ptr<node>>& nod, attribute* att );
func_ptr	eat_tag_attr_name( char currCh, document* doc, vector<shared_ptr<node>>& nod, attribute* att );
func_ptr	eat_tag_attr_value( char currCh, document* doc, vector<shared_ptr<node>>& nod, attribute* att );
func_ptr	eat_tag_attr_value_quoted( char currCh, document* doc, vector<shared_ptr<node>>& nod, attribute* att );
func_ptr	eat_tag_attr_value_unquoted( char currCh, document* doc, vector<shared_ptr<node>>& nod, attribute* att );
func_ptr	eat_text_chars( char currCh, document* doc, vector<shared_ptr<node>>& nod, attribute* att );
func_ptr	eat_entity_chars( char currCh, document* doc, vector<shared_ptr<node>>& nod, attribute* att );


func_ptr	eat_text_chars( char currCh, document* doc, vector<shared_ptr<node>>& nod, attribute* att )
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


func_ptr	eat_entity_chars( char currCh, document* doc, vector<shared_ptr<node>>& nod, attribute* att )
{
	static std::string	sEntityName;
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
			theNode->text.append(sEntities[sEntityName]);
			sEntityName.erase();
			return (func_ptr)eat_text_chars;
			break;
		}
		
		default:
		{
			sEntityName.append(1,currCh);
			return (func_ptr)eat_entity_chars;
			break;
		}
	}
	
	return nullptr;
}


func_ptr	eat_whitespace( char currCh, document* doc, vector<shared_ptr<node>>& nod, attribute* att )
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
			return eat_text_chars( currCh, doc, nod, att );
			break;
	}
	
	return nullptr;
}


func_ptr	eat_tag_name( char currCh, document* doc, vector<shared_ptr<node>>& nod, attribute* att )
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


func_ptr	eat_tag_attr_name_whitespace( char currCh, document* doc, vector<shared_ptr<node>>& nod, attribute* att )
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


func_ptr	eat_tag_attr_name( char currCh, document* doc, vector<shared_ptr<node>>& nod, attribute* att )
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


func_ptr	eat_tag_attr_value( char currCh, document* doc, vector<shared_ptr<node>>& nod, attribute* att )
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


func_ptr	eat_tag_attr_value_quoted( char currCh, document* doc, vector<shared_ptr<node>>& nod, attribute* att )
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
		
		default:
			att->value.append( 1, currCh );
			return (func_ptr)eat_tag_attr_value_quoted;
			break;
	}
	
	return nullptr;
}


func_ptr	eat_tag_attr_value_unquoted( char currCh, document* doc, vector<shared_ptr<node>>& nod, attribute* att )
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


document::document( const char* inString, size_t inLength )
{
	vector<shared_ptr<node>>	nod;
	nod.push_back( shared_ptr<node>(new node) );
	attribute					att;
	eat_char_fcn				state = eat_whitespace;
	size_t						x = 0;
	while( (x < inLength) && state )
	{
		state = (eat_char_fcn)state( inString[x++], this, nod, &att );
	}
	
	nod.back()->print();
}


void	node::print()
{
	for( auto child : children )
		child->print();
}



void	tag::print()
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
		
		node::print();
		
		cout << "</" << name << ">";
	}
}


void	text::print()
{
	cout << "«" << text << "»";
}

