'use strict' ;

// decompress_alignment takes in a reference sequence (encoded as Uint8Array) and
// a compressed sequence (also Uint8Array).
// The compressed sequence is assumed to be made up of sequences of bases, and sequences of numbers.
// Currently these are encoded in ASCII i.e. values 48-57 indicate digits.
// A number means 'copy this many bases from the reference sequence'.
// A non-number means 'insert this base verbatim'.
// This therefore implements a simple form of decompression that can help reduce file sizes.
function decompress_alignment( reference, sequence ) {
	let L = reference.length ;
	let uncompressed_sequence = new Uint8Array( L ) ;
	let state = {
		type: null,
		pos: 0,
		length: 0
	} ;
	let j = 0 ;
	let out_j = 0 ;
	for( ; j < sequence.length; ++j ) {
		// 48 is ASCII for 0
		// 57 is ASCII for 9
		let type = (sequence[j] >= 48 & sequence[j] < 58) ? 'number' : 'sequence' ;
		if( type == 'number' ) {
			if( (!state.type) || state.type == 'sequence' ) {
				state.length = 0 ;
			}
			state.length *= 10 ;
			state.length += ( sequence[j] - 48 ) ;
			state.type = 'number' ;
		} else if( type == 'sequence' ) {
			if( state.type == 'number' ) {
				for( let k = 0; k < state.length; ++k, ++state.pos ) {
					uncompressed_sequence[state.pos] = reference[state.pos] ;
				}
			}
			uncompressed_sequence[state.pos++] = sequence[j] ;
			state.type = 'sequence' ;
		}
	}
	if( state.type == 'number' ) {
		for( let k = 0; k < state.length; ++k, ++state.pos ) {
			uncompressed_sequence[state.pos] = reference[state.pos] ;
		}
	}
	assert( state.pos == L ) ;
	return uncompressed_sequence ;
}

function unpack_alignments( alignment ) {
	if( alignment.length == 0 ) {
		return alignment ;
	}
	let encoder = new TextEncoder() ;
	alignment[0].sequence = alignment[0].sequence.toLowerCase() ;
	alignment[0].sequence = encoder.encode( alignment[0].sequence ) ;
	for( let i = 1; i < alignment.length; ++i ) {
		alignment[i].sequence = encoder.encode( alignment[i].sequence.toLowerCase() ) ;
		alignment[i].sequence = decompress_alignment(
			alignment[0].sequence,
			alignment[i].sequence
		) ;
	}
	// console.log( alignment ) ;
	return alignment ;
}

function run_msa_viewer( data ) {
	// Sequences are unpacked and turned into 
	data.alignment = unpack_alignments( data.alignment ) ;

	data.genes = data.genes ? data.genes : [] ;
	data.annotations = data.annotations ? data.annotations : {} ;
	data.highlights = data.highlights ? data.highlights : [] ;

	let reference_name = data.alignment[ 0 ].name ;
	let msa = new MSA( data.alignment, data.ranges ) ;
	let reference = msa.scales.ungappedSequences[reference_name] ;
	console.log( "REFERENCE", reference ) ;
	let sequence_type = data.sequence_type ? data.sequence_type : "dna" ;
	let viewer = new MSAView(
		d3.select( ".figure" ),
		msa,
		reference,
		sequence_type,
		new GeneView( data.genes, reference ),
		data.annotations,
		themes[sequence_type]
	) ;
	let controller = new MSAController(
		viewer.panels,
		viewer,
		new URLSearchParams(window.location.search)
	) ;

	let redraw = function() {
		viewer.draw() ;
		window.requestAnimationFrame( redraw ) ;
	} ;
	window.requestAnimationFrame( redraw ) ;
}
