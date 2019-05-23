#include <iostream>
#include <string>
#include <fstream>
#include <cstdint>
#include <map>
#include <algorithm>
#include <iomanip>

using namespace std;

void make_marie( map<string,uint16_t> & cmds );
void read_labels( map<string,uint16_t> const & cmds, map<string,uint16_t> & lbls, ifstream & in );

int main( int argc, char ** argv ) {

	if( argc < 2 ) {
		cout << "no file" << endl;
		return -1;
    }

	ifstream in( argv[1] );
	ofstream out( "out.bin", ios::binary );

	if( !in.is_open() ) return -2;
	if( !out.is_open() ) return -3;

	map<string,uint16_t> cmds;
	map<string,uint16_t> lbls;

	make_marie( cmds );
	read_labels( cmds, lbls, in );

	in.clear();
	in.seekg( 0, ios::beg );

	string in_word;
	size_t addr = 0;
	uint16_t op = 0;
	uint16_t pc = 0;

	cout << setfill( '0' );

	while( in >> in_word ) {
		string word = "";
		for( size_t i = 0; i<in_word.size(); i++ ) word += tolower( in_word.at( i ) );
		if( cmds.find( word ) == cmds.end() ) {
			if( lbls.find( in_word ) != lbls.end() ) continue;
			try {
				op = stoi( word, nullptr, 16 );
				cout << setw( 4 ) << hex << op;
				out.write( (char*) &op, 2 );
				pc++;
				continue;
			}
			catch( logic_error err ) {
				cout << endl << "unknown instruction at " << pc << ": " << in_word << endl;
				return 1;
			}
		}
		op = cmds.at( word );
		if( op & 0x0fff ) {
			if( !(in >> hex >> addr) ) {
				in.clear();
				string arg;
				in >> arg;
				if( lbls.find( arg ) == lbls.end() ) {
					cout << endl << "unknown label at " << pc << ": " << arg << endl;
					return 2;
				}
				addr = lbls.at( arg );
			}
			op &= 0xf000;
			op |= addr;
		}
		out.write( (char*) &op, 2 );
		cout << setw( 4 ) << hex << op;
		pc++;
	}

	in.close();
	out.close();

    return 0;
}

void make_marie( map<string,uint16_t> & cmds ) {
	cmds.emplace( "add", 0x3fff );
	cmds.emplace( "subt", 0x4fff );
	cmds.emplace( "addl", 0xbfff );
	cmds.emplace( "clear", 0xa000 );
	cmds.emplace( "load", 0x1fff );
	cmds.emplace( "store", 0x2fff );
	cmds.emplace( "input", 0x5000 );
	cmds.emplace( "output", 0x6000 );
	cmds.emplace( "jump", 0x9fff );
	cmds.emplace( "skipcond", 0x8fff );
	cmds.emplace( "jns", 0x0fff );
	cmds.emplace( "jumpl", 0xcfff );
	cmds.emplace( "halt", 0x7000 );
}

void read_labels( map<string,uint16_t> const & cmds, map<string,uint16_t> & lbls, ifstream & in ) {
	string in_word;
	size_t pc = 0;
	while( in >> in_word ) {
		string word = "";
		for( size_t i = 0; i<in_word.size(); i++ ) word += tolower( in_word.at( i ) );
		if( cmds.find( word ) != cmds.end() ) { if( cmds.at( word ) & 0x0fff ) in >> word; }
		else {
			if( !lbls.emplace( in_word, pc ).second ) {
				cout << "duplicate label at " << pc << ": " << in_word << endl;
				exit( 3 );
			}
			else cout << "registered label " << in_word << " as " << pc << endl;
			in >> in_word;
		}
		pc++;
	}
}
