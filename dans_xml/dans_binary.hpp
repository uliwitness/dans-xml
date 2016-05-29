//
//  dans_binary.hpp
//  dans_xml
//
//  Created by Uli Kusterer on 28/05/16.
//  Copyright © 2016 Uli Kusterer. All rights reserved.
//

#ifndef dans_binary_hpp
#define dans_binary_hpp

#include "dans_xml.hpp"
#include <map>


namespace dans_xml
{
	enum type
	{
		BOOLEAN_TAG_FALSE,	// <false /> represented by its own type.
		BOOLEAN_TAG_TRUE,	// <true /> represented by its own type.
		BOOLEAN_FALSE,		// false represented by its own type.
		BOOLEAN_TRUE,		// true represented by its own type.
		SINT64,				// 64-bit signed integer.
		NEWSTRING8,			// Initial definition of string. Append to table. Size of string is a 8-bit unsigned integer.
		STRING8,			// String by table index specified in an 8-bit unsigned integer.
		NEWEMPTYTAG8,		// Tag plus initial definition of the string that is its name. Size of string is a 8-bit unsigned integer.
		EMPTYTAG8,			// Tag by string table index specified in an 8-bit unsigned integer.
		NEWTAG8,			// Tag plus initial definition of the string that is its name. Size of string is a 8-bit unsigned integer. Followed by a 8-bit unsigned integer count and that many attributes.
		TAG8,				// String by string table index specified in an 8-bit unsigned integer. Followed by a 8-bit unsigned integer count and that many attributes.
	};
	typedef uint8_t	data_type;
	
	
	class binary_writer : public writer
	{
	public:
		binary_writer( FILE* outFile );
		
		virtual	void	write_integer( long long inNum, size_t depth ) override;
		virtual	void	write_bool( bool inValue, size_t depth ) override;
		virtual	void	write_string( const std::string& inStr, size_t depth ) override;
		virtual void	write_open_tag_before_attributes( const std::string& inTagName, size_t numAttributes, size_t numChildren, size_t depth ) override;
		virtual void	write_open_tag_after_attributes( const std::string& inTagName, size_t numAttributes, size_t numChildren, size_t depth ) override;
		virtual void	write_attribute( const std::string& inName, const std::string& inValue ) override;
		virtual void	write_close_tag( const std::string& inTagName, size_t numChildren, size_t depth ) override;

		size_t			index_for_string( const std::string& inStr, bool *isNew );
		template<class T>
		void			write_typed( T inNum );

		std::map<std::string,size_t>	stringTable;
		size_t							stringTableIDSeed;
		FILE*							file;
	};
	
	class binary_reader
	{
	public:
		binary_reader( document& inDoc, FILE* inFile );
		
		bool		read_one_tag( std::shared_ptr<node> parent, data_type tagType );	// Caller must read_typed<data_type>() the tagType before calling in!
		bool		read_one_attribute( std::shared_ptr<tag> parent );
		std::string	read_one_string( bool *outSuccess, data_type tagType );
		bool		read_one_tag_or_string( std::shared_ptr<node> parent, data_type tagType );
		
		template<class T>
		T	read_typed();

		FILE*						file;
		document&					doc;
		std::vector<std::string>	stringTable;
	};
}

#endif /* dans_binary_hpp */
