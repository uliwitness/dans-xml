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


template<class T>
void	binary_writer::write_typed( T inNum )
{
	fwrite( &inNum, sizeof(inNum), 1, file );
}


void	binary_writer::write_integer( long long inNum, size_t depth )
{
	write_typed<data_type>( SINT64 );
	write_typed<int64_t>( inNum );
}


void	binary_writer::write_bool( bool inValue, size_t depth )
{
	write_typed<uint8_t>( inValue ? BOOLEAN_TRUE : BOOLEAN_FALSE );
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
	write_typed<typeof(type)>(type);
	if( !isNew )
	{
		write_typed<uint8_t>(idx);
	}
	else
	{
		size_t	strLen = inStr.size();
		write_typed<uint8_t>(strLen);
		fwrite( inStr.data(), 1, strLen, file );
	}
}


void	binary_writer::write_open_tag_before_attributes( const std::string& inTagName, size_t numAttributes, size_t numChildren, size_t depth )
{
	if( inTagName.compare("true") == 0 && numAttributes == 0 && numChildren == 0 )
	{
		write_typed<uint8_t>( BOOLEAN_TAG_TRUE );
	}
	else if( inTagName.compare("false") == 0 && numAttributes == 0 && numChildren == 0 )
	{
		write_typed<uint8_t>( BOOLEAN_TAG_FALSE );
	}
	else
	{
		bool	isNew = false;
		size_t	idx = index_for_string( inTagName, &isNew );
		uint8_t	type = isNew ? NEWTAG8 : TAG8;
		if( numAttributes == 0 )
			type = isNew ? NEWEMPTYTAG8 : EMPTYTAG8;
		write_typed<uint8_t>(type);
		if( !isNew )
		{
			write_typed<uint8_t>(idx);
		}
		else
		{
			uint8_t	strLen = inTagName.size();
			write_typed<uint8_t>(strLen);
			fwrite( inTagName.data(), 1, strLen, file );
		}
		if( numAttributes > 0 )
		{
			write_typed<uint8_t>(numAttributes);
		}
	}
}


void	binary_writer::write_open_tag_after_attributes( const std::string& inTagName, size_t numAttributes, size_t numChildren, size_t depth )
{
	write_typed<uint8_t>(numChildren);
}


void	binary_writer::write_attribute( const std::string& inName, const std::string& inValue )
{
	write_string( inName, 0 );
	write_string( inValue, 0 );
}


void	binary_writer::write_close_tag( const std::string& inTagName, size_t numChildren, size_t depth )
{
	
}


binary_reader::binary_reader( document& inDoc, FILE* inFile )
: doc(inDoc), file(inFile)
{
	// Start of our table contains some default strings that likelt every document will contain. No need to write those to the file:
#define X(n)	stringTable.push_back(n);
	INITIAL_STRING_TABLE
#undef X
	
	fseek( file, 0, SEEK_END);
	long	pos = ftell(file);
	rewind(file);

	uint8_t	numDefaults = read_typed<uint8_t>();
	if( numDefaults > stringTable.size() )
		return;	// Newer format, don't know what to use for the new string keys.
	if( numDefaults < stringTable.size() )
		stringTable.resize(numDefaults);// Discard the newer ones we know, so the document-defined indexes match.
	
	inDoc.root = make_shared<node>();
	while( ftell(file) != pos )
		read_one_tag( inDoc.root, read_typed<data_type>() );
}


template<class T>
T	binary_reader::read_typed()
{
	T	type = 0;
	fread( &type, sizeof(type), 1, file );
	return type;
}


std::string	binary_reader::read_one_string( bool *outSuccess, data_type tagType )
{
	string	tagName;
	
	*outSuccess = true;
	
	switch( tagType )
	{
		case NEWSTRING8:
		{
			uint8_t	len = read_typed<uint8_t>();
			if( len == 1 )
				printf("");
			tagName.resize(len);
			fread( &(tagName[0]), 1, len, file );
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
	string	name = read_one_string( &success, read_typed<data_type>() );
	if( !success )
		return false;
	string	value = read_one_string( &success, read_typed<data_type>() );
	if( !success )
		return false;
	parent->set_attribute( name, value );
	return true;
}


bool	binary_reader::read_one_tag_or_string( shared_ptr<node> parent, data_type tagType )
{
	switch( tagType )
	{
		case BOOLEAN_FALSE:
			return false; // TODO
			break;
			
		case BOOLEAN_TRUE:
			return false; // TODO
			break;
			
		case SINT64:
			return false; // TODO
			break;
			
		case NEWSTRING8:
		case STRING8:
		{
			bool	success = false;
			shared_ptr<text>	theText = make_shared<text>();
			theText->actualText = read_one_string( &success, tagType );
			if( success )
				parent->children.push_back( theText );
			return success;
			break;
		}
			
		case NEWEMPTYTAG8:
		case EMPTYTAG8:
		case NEWTAG8:
		case TAG8:
		case BOOLEAN_TAG_FALSE:
		case BOOLEAN_TAG_TRUE:
			read_one_tag( parent, tagType );
			break;
		default:
			return false;
	}
	
	return true;
}


bool	binary_reader::read_one_tag( shared_ptr<node> parent, data_type tagType )
{
	string	tagName;
	size_t	numAttributes = 0;
	size_t	numChildren = 0;
	switch( tagType )
	{
		case NEWEMPTYTAG8:
		{
			uint8_t	len = read_typed<uint8_t>();
			tagName.resize(len);
			fread( &(tagName[0]), 1, len, file );
			stringTable.push_back(tagName);
			break;
		}
		case NEWTAG8:
		{
			uint8_t	len = read_typed<uint8_t>();
			tagName.resize(len);
			fread( &(tagName[0]), 1, len, file );
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
		case BOOLEAN_TAG_TRUE:
			tagName = "true";
			break;
		case BOOLEAN_TAG_FALSE:
			tagName = "false";
			break;
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
		if( !read_one_tag_or_string( newTag, read_typed<data_type>() ) )
			return false;
	}
	
	return true;
}

