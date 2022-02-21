'use strict';

let computeUngappedSequence = function( gappedSequence, coordinateRange ) {
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
	for( let i = 0; i < gappedSequence.length; ++i ) {
		if( gappedSequence[i] == "-" && inRange ) {
			range.inAlignment.end = i ;
			range.inSequence.end = coordinateRange.start + ungappedSequence.length ;
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
			range.inSequence.start = coordinateRange.start + ungappedSequence.length ;
			inRange = true ;
			if( ranges.length == 0 ) {
				firstAlignmentBase = i ;
			}
		}
		if( gappedSequence[i] != "-" ) {
			ungappedSequence.push( gappedSequence[i] ) ;
		}
	}
	//console.log( "S", gappedSequence, ungappedSequence ) ;
	// sanity check
	assert(
		ungappedSequence.length == (coordinateRange.end - coordinateRange.start),
		(
			"!! ERROR: computeUngappedSequence(): sequence length (" +
			ungappedSequence.length +
			") does not match coordinate range (" +
			coordinateRange.start + " - " + coordinateRange.end +
			 " (length " + (coordinateRange.end - coordinateRange.start) + ")."
		)
	) ;
	
	return {
		sequence: ungappedSequence,
		ranges: ranges,
		coordinateRange: coordinateRange
	} ;
} ;

// scales that map sequence coords to MSA coords
let MSAScales = function( alignment, coordinateRanges ) {
	this.alignment = alignment ;
	this.alignmentLength = alignment[0].sequence.length ;
	this.ungappedSequences = {} ;
	this.ranges = {} ;
	
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

