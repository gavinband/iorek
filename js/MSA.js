'use strict';

let computeUngappedSequence = function( name, gappedSequence, coordinateRange ) {
	let ranges = [] ;
	let range = {
		inAlignment: {start: 0, end: 0 },
		inSequence: {start: 0, end: 0 }
	} ;
	let firstAlignmentBase = 0 ;
	let lastAlignmentBase = 0 ;
	let inRange = false ;
	let sequencePosition = 0 ;
	let ungappedSequence = [] ;
	let orientation = Math.sign( coordinateRange.end - coordinateRange.start ) ;
	for( let i = 0; i < gappedSequence.length; ++i ) {
		if( gappedSequence[i] == "-" && inRange ) {
			range.inAlignment.end = i ;
			range.inSequence.end = coordinateRange.start + orientation * ungappedSequence.length ;
			// take care here to push a newly-built range object otherwise
			// they all refer to the same one.
			ranges.push( {
				inAlignment: {
					start: range.inAlignment.start,
					end: range.inAlignment.end
				},
				inSequence: {
					start: range.inSequence.start,
					end: range.inSequence.end
				}
			} ) ;
			inRange = false ;
			lastAlignmentBase = i ;
		} else if( gappedSequence[i] != "-" && !inRange ) {
			range.inAlignment.start = i ;
			range.inSequence.start = coordinateRange.start + orientation * ungappedSequence.length ;
			inRange = true ;
			if( ranges.length == 0 ) {
				firstAlignmentBase = i ;
			}
		}
		if( gappedSequence[i] != "-" ) {
			ungappedSequence.push( gappedSequence[i] ) ;
		}
	}
	
	if( inRange ) {
		range.inAlignment.end = gappedSequence.length ;
		range.inSequence.end = coordinateRange.start + orientation * ungappedSequence.length ;
		// take care here to push a newly-built range object otherwise
		// they all refer to the same one.
		ranges.push( {
			inAlignment: {
				start: range.inAlignment.start,
				end: range.inAlignment.end
			},
			inSequence: {
				start: range.inSequence.start,
				end: range.inSequence.end
			}
		} ) ;
	}
	
	//console.log( "S", gappedSequence, ungappedSequence ) ;
	// sanity check
	assert(
		ungappedSequence.length == Math.abs(coordinateRange.end - coordinateRange.start)+1,
		(
			"!! ERROR: computeUngappedSequence(): (\"" + name + "\"): sequence length (" +
			ungappedSequence.length +
			") does not match coordinate range (" +
			coordinateRange.start + " - " + coordinateRange.end +
			 " (length " + (Math.abs(coordinateRange.end - coordinateRange.start)+1) + ")."
		)
	) ;
	
	return {
		sequence: ungappedSequence,
		ranges: ranges,
		coordinateRange: coordinateRange
	} ;
} ;

let preprocessAlignment = function( alignment ) {
	// process the MSA to add additional detail tracks
	// this includes mismatches track and variable level summaries.
	for( let i = 0; i < alignment.length; ++i ) {
		alignment[i].mismatches = [...alignment[i].sequence] ;
	}
	for( let j = 0; j < alignment[0].sequence.length; ++j ) {
		let mismatch = false ;
		let ref = -1 ;
		for( let i = 0; i < alignment.length; ++i ) {
			let base = alignment[i].sequence[j] ;
			if( base != '-' ) {
				if( ref == -1 ) {
					ref = i ;
				} else if( base != alignment[ref].sequence[j] ) {
					mismatch = true ;
					break ;
				}
			}
		}
		if( !mismatch ) {
			for( let i = 0; i < alignment.length; ++i ) {
				if( alignment[i].mismatches[j] != "-" ) {
					alignment[i].mismatches[j] = "m" ;
				}
			}
		}
	}

	for( let i = 0; i < alignment.length; ++i ) {
		alignment[i].sequence.levels = computeLevels(
			alignment[i].sequence,
			[ "-", "a", "c", "g", "t" ],
			function( counts ) {
				// Compute the most frequent base/char in each window,
				// resolving ties based on the supplied order.
				// This is ad hoc but makes visualisation simple.
				let max = 0 ;
				let max_base = '-' ;
				[ "-", "a", "c", "g", "t" ].forEach( function(x) {
					if( counts[x] >= max ) {
						max = counts[x] ;
						max_base = x ;
					}
				})
				return max_base ;
			}
		) ;
		alignment[i].mismatches.levels = computeLevels(
			alignment[i].mismatches,
			[ "-", "m", "a", "c", "g", "t" ],
			function( counts ) {
				let max = 0 ;
				let max_base = '-' ;
				[ 'a', 'c', 'g', 't' ].forEach( function(x) {
					if( counts[x] >= max ) {
						max = counts[x] ;
						max_base = x ;
					}
				}) ;
				if( max == 0 ) {
					[ '-', 'm' ].forEach( function(x) {
						if( counts[x] >= max ) {
							max = counts[x] ;
							max_base = x ;
						}
					}) ;
				}
				return max_base ;
			}
		) ;
	}
	
	return alignment ;
}

function computeLevels(
	sequence,
	what = [ "A", "C", "G", "T", "-" ],
	summarise,
	levels = [ 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024 ]
) {
	let result = [] ;
	result[1] = [[ ...sequence ]] ;
	let i = 0;
	for( let level_i = 0; level_i < levels.length; ++level_i ) {
		let level = levels[level_i] ;
		result[level] = [] ;
		let counts = {} ;
		what.forEach( x => counts[x] = 0 ) ;
		for( i = 0; i < sequence.length; ++i ) {
			if( i % level == 0 ) {
				if( i > 0 ) {
					result[level].push( summarise(counts) ) ;
				}
				what.forEach( x => counts[x] = 0 ) ;
			}
			++counts[ sequence[i].toLowerCase() ] ;
		}
		if( i % level != 0 ) {
			result[level].push( summarise(counts) ) ;
		}
	}
	return result ;
}

// scales that map sequence/contig coords to MSA coords
let MSAScales = function( alignment, coordinateRanges ) {
	this.alignment = preprocessAlignment( alignment ) ;
	this.alignmentLength = alignment[0].sequence.length ;
	this.ungappedSequences = {} ;
	this.ranges = {} ;

	console.log( "MSAScales", this.alignment ) ;
	for( let i = 0; i < alignment.length; ++i ) {
		let gappedSequence = alignment[i].sequence ;
		assert(
			gappedSequence.length == this.alignmentLength,
			"!! MSAScales(): error: length of sequence "
				+ i + " (" + gappedSequence.length
				+ ") does not match length of first sequence ("
				+ this.alignmentLength +" )"
		) ;
		let ungapped = computeUngappedSequence(
			alignment[i].name,
			gappedSequence,
			coordinateRanges[ alignment[i].name ]
		) ;
		ungapped.name = alignment[i].name ;
		let scales = [] ;
		for( let j = 0; j < ungapped.ranges.length; ++j ) {
			let range = ungapped.ranges[j] ;
			scales.push(
				new d3.scaleLinear()
					.domain( [ range.inSequence.start, range.inSequence.end ] )
					.range( [ range.inAlignment.start, range.inAlignment.end ] )
			) ;
		}
		this.ranges[alignment[i].name] = scales ;
		this.ungappedSequences[alignment[i].name] = ungapped ;
	}
	this.global = d3.scaleLinear()
		.domain( [0, this.alignmentLength] )
		.range( [0, this.alignmentLength] )
	;
	console.log( "MSAScales()", alignment ) ;
	
	return this ;
} ;

let MSA = function( alignment, coordinateRanges ) {
	this.alignment = alignment ;
	this.coordinateRanges = coordinateRanges ;
	this.n = alignment.length ;
	this.scales = new MSAScales( alignment, coordinateRanges )
	this.alignmentLength = this.scales.alignmentLength ;
	return this ;
} ;

MSA.prototype.sequence_ids = function() {
	return this.alignment.map( elt => elt.name ) ;
}

