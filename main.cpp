#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <fstream>
#include <stdint.h>
#include <experimental/filesystem>

typedef int8_t   i8;
typedef int16_t  i16;
typedef int32_t  i32;
typedef int64_t  i64;

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

int main( )
{
	std::vector< std::string >	directory_entries;
	std::vector< u8 >			data;
	std::fstream				file;
	std::string					selected_file, input;

	std::cout.setf( std::ios::fixed );;
	std::cout.precision( 2 );

	// find each file in the current directory, add them to our list
	for ( auto& file : std::experimental::filesystem::directory_iterator( std::experimental::filesystem::current_path( ) ) )
	{
		if ( !std::experimental::filesystem::is_regular_file( file ) )
			continue;

		std::string path = file.path( ).string( );

		const u32 pos = path.find_last_of( '\\' );
		if ( pos != std::string::npos )
		{
			// cull to just filename
			path.erase( 0, pos + 1 );

			for ( auto& letter : path )
				letter = std::tolower( letter );

			directory_entries.push_back( path );
		}
	}

	// print list and let the user select an entry
	{
		std::cout << std::experimental::filesystem::current_path( ) << ':' << std::endl;
		for ( u32 i = 0; i < directory_entries.size( ); ++i )
			std::cout << i + 1 << ". " << directory_entries[std::min( i + 1, directory_entries.size( ) - 1 )] << std::endl;

		// poll user input to search the list
		std::cin >> input;

		// just a number?
		if ( input.find_first_not_of( "1234567890" ) == std::string::npos )		
			selected_file = directory_entries[std::clamp( std::atoi( input.c_str( ) ), 0, ( int ) directory_entries.size( ) - 1 )];		

		// nop, search list and see if there's a similar string
		else for ( auto& entry : directory_entries )
		{
			if ( entry.find( input ) == std::string::npos )
				continue;

			selected_file = entry;
			break;
		}

		// bad typeings ?_?
		if ( selected_file.empty( ) )
		{
			std::cout << "failed to find '" << input << "'" << std::endl;
			system( "pause" );
			return 0;
		}
	}

	// load file into 'data' vector
	{
		file.open( selected_file, std::ios::in | std::ios::binary );

		file.seekg( 0, std::ios::end );
		data.reserve( file.tellg( ) );
		file.seekg( 0, std::ios::beg );

		std::copy( std::istreambuf_iterator< char >( file ), std::istreambuf_iterator< char >( ), std::back_inserter( data ) );

		file.close( );

		std::cout << std::endl << selected_file << "(" << ( float ) data.size( ) / 1024.0f << "kb)... ";
	}

	// output to file
	file.open( "_" + selected_file + ".txt", std::ios::out | std::ios::trunc );
	{
		std::replace_if( selected_file.begin( ), selected_file.end( ), [=]( char a ) { return iswspace( a ) || ispunct( a ); }, '_' );

		file << "unsigned char" << ( isdigit( selected_file.front( ) ) ? '_' : ' ' ) << selected_file << "[" << data.size( ) << "]" << " {" << std::endl;

		for ( u32 i = 0; i < data.size( ); ++i )
		{
			file << "0x" << std::hex << ( data[i] & 0xff ) << ",";

			if ( ( i + 1 ) % 10 == 0 )
				file << std::endl;
		}

		file << std::endl << "};";
		file.close( );
	}

	std::cout << "finished" << std::endl;
	system( "pause" );
}