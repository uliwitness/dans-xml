//
//  dans_binary.cpp
//  dans_xml
//
//  Created by Uli Kusterer on 28/05/16.
//  Copyright © 2016 Uli Kusterer. All rights reserved.
//

#include "dans_binary.hpp"


using namespace std;
using namespace dans_xml;


#define INITIAL_STRING_TABLE	X("") \
								X("?xml") \
								X("version") \
								X("encoding") \
								X("utf-8") \
								X("!DOCTYPE") \
								X("PUBLIC") \
								X("id") \
								X("name")


binary_writer::binary_writer( FILE* outFile )
	: stringTableIDSeed(0), file(outFile)
{
	// Start of our table contains some default strings that likelt every document will contain. No need to write those to the file:
	#define X(n)	stringTable[n] = stringTableIDSeed++;
	INITIAL_STRING_TABLE
	#undef X
	
	// Write number of default strings to file. Kinda as a file format.
	//	Only readers who know these defaults can parse a file, after all.
	uint8_t	type = stringTable.size();
	fwrite( &type, sizeof(type), 1, file );
}


void	binary_writer::write_integer( long long inNum, size_t depth )
{
	uint8_t	type = SINT64;
	fwrite( &type, sizeof(type), 1, file );
	int64_t	num = inNum;
	fwrite( &num, sizeof(num), 1, file );
}


void	binary_writer::write_bool( bool inValue, size_t depth )
{
	uint8_t	type = inValue ? BOOLEAN_TRUE : BOOLEAN_FALSE;
	fwrite( &type, sizeof(type), 1, file );
}


size_t	binary_writer::index_for_string( const std::string& inStr, bool *isNew )
{
	size_t	idx = 0;
	std::map<std::string,size_t>::iterator foundStr = stringTable.find(inStr);
	if( foundStr != stringTable.end() )
	{
		idx = foundStr->second;
		*isNew = false;
	}
	else
	{
		idx = stringTableIDSeed++;
		stringTable[inStr] = idx;
		*isNew = true;
	}
	return idx;
}


void	binary_writer::write_string( const std::string& inStr, size_t depth )
{
	bool	isNew = false;
	size_t	idx = index_for_string( inStr, &isNew );
	uint8_t	type = isNew ? NEWSTRING8 : STRING8;
	fwrite( &type, sizeof(type), 1, file );
	if( !isNew )
	{
		uint8_t	strIndex = idx;
		fwrite( &strIndex, sizeof(strIndex), 1, file );
	}
	else
	{
		uint8_t	strLen = inStr.size();
		fwrite( &strLen, sizeof(strLen), 1, file );
		fwrite( inStr.data(), 1, strLen, file );
	}
}


void	binary_writer::write_open_tag_before_attributes( const std::string& inTagName, size_t numAttributes, size_t numChildren, size_t depth )
{
	bool	isNew = false;
	size_t	idx = index_for_string( inTagName, &isNew );
	uint8_t	type = isNew ? NEWTAG8 : TAG8;
	if( numAttributes == 0 )
		type = isNew ? NEWEMPTYTAG8 : EMPTYTAG8;
	fwrite( &type, sizeof(type), 1, file );
	if( !isNew )
	{
		uint8_t	strIndex = idx;
		fwrite( &strIndex, sizeof(strIndex), 1, file );
	}
	else
	{
		uint8_t	strLen = inTagName.size();
		fwrite( &strLen, sizeof(strLen), 1, file );
		fwrite( inTagName.data(), 1, strLen, file );
	}
	if( numAttributes > 0 )
	{
		uint8_t	na = numAttributes;
		fwrite( &na, sizeof(na), 1, file );
	}
}


void	binary_writer::write_open_tag_after_attributes( const std::string& inTagName, size_t numAttributes, size_t numChildren, size_t depth )
{
	uint8_t	nc = numChildren;
	fwrite( &nc, sizeof(nc), 1, file );
}


void	binary_writer::write_attribute( const std::string& inName, const std::string& inValue )
{
	write_string( inName, 0 );
	write_string( inValue, 0 );
}


void	binary_writer::write_close_tag( const std::string& inTagName, size_t numChildren, size_t depth )
{
	
}


template<class T>
T	binary_reader::read_typed()
{
	T	type = 0;
	fread( &type, sizeof(type), 1, file );
	return type;
}


std::string	binary_reader::read_one_string( bool *outSuccess )
{
	string	tagName;
	
	*outSuccess = true;
	
	switch( read_typed<uint8_t>() )
	{
		case NEWSTRING8:
		{
			uint8_t	len = read_typed<uint8_t>();
			tagName.resize(len);
			fread( &(tagName[0]), 1, 1, file );
			stringTable.push_back(tagName);
			break;
		}
		case STRING8:
		{
			uint8_t	idx = read_typed<uint8_t>();
			tagName = stringTable[idx];
			break;
		}
		default:
			*outSuccess = false;	// Invalid file format.
	}
	
	return tagName;
}


bool	binary_reader::read_one_attribute( shared_ptr<tag> parent )
{
	bool	success = true;
	string	name = read_one_string( &success );
	if( !success )
		return false;
	string	value = read_one_string( &success );
	if( !success )
		return false;
	parent->set_attribute( name, value );
	return true;
}


bool	binary_reader::read_one_tag( shared_ptr<node> parent )
{
	string	tagName;
	size_t	numAttributes = 0;
	size_t	numChildren = 0;
	switch( read_typed<uint8_t>() )
	{
		case NEWEMPTYTAG8:
		{
			uint8_t	len = read_typed<uint8_t>();
			tagName.resize(len);
			fread( &(tagName[0]), 1, 1, file );
			stringTable.push_back(tagName);
			break;
		}
		case NEWTAG8:
		{
			uint8_t	len = read_typed<uint8_t>();
			tagName.resize(len);
			fread( &(tagName[0]), 1, 1, file );
			stringTable.push_back(tagName);
			numAttributes = read_typed<uint8_t>();
			break;
		}
		case EMPTYTAG8:
		{
			uint8_t	idx = read_typed<uint8_t>();
			tagName = stringTable[idx];
			break;
		}
		case TAG8:
		{
			uint8_t	idx = read_typed<uint8_t>();
			tagName = stringTable[idx];
			numAttributes = read_typed<uint8_t>();
			break;
		}
		default:
			return false;	// Invalid file format.
	}
	
	shared_ptr<tag>	newTag = make_shared<tag>( tagName );
	parent->children.push_back( newTag );

	for( size_t x = 0; x < numAttributes; x++ )
	{
		if( !read_one_attribute( newTag ) )
			return false;
	}
	
	numChildren = read_typed<uint8_t>();

	for( size_t x = 0; x < numChildren; x++ )
	{
		if( !read_one_tag( newTag ) )
			return false;
	}
	
	return true;
}


binary_reader::binary_reader( document& inDoc, FILE* inFile )
	: doc(inDoc), file(inFile)
{
	// Start of our table contains some default strings that likelt every document will contain. No need to write those to the file:
	#define X(n)	stringTable.push_back(n);
	INITIAL_STRING_TABLE
	#undef X
	
	uint8_t	numDefaults = read_typed<uint8_t>();
	if( numDefaults > stringTable.size() )
		return;	// Newer format, don't know what to use for the new string keys.
	if( numDefaults < stringTable.size() )
		stringTable.resize(numDefaults);// Discard the newer ones we know, so the document-defined indexes match.
	
	inDoc.root = shared_ptr<node>(new node);
	read_one_tag( inDoc.root );
}

