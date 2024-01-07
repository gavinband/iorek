"use strict";

let reverse_complement_map = {
	97: 116,
	116: 97,
	99: 103,
	103: 99,
	110: 110,
	45: 45,
	109: 109,
	120: 120
} ;

let aa_table = {
	'aaa': 'Lys',
	'aac': 'Asn',
	'aag': 'Lys',
	'aat': 'Asn',
	'aca': 'Thr',
	'acc': 'Thr',
	'acg': 'Thr',
	'act': 'Thr',
	'aga': 'Arg',
	'agc': 'Ser',
	'agg': 'Arg',
	'agt': 'Ser',
	'ata': 'Ile',
	'atc': 'Ile',
	'atg': 'Met',
	'att': 'Ile',
	'caa': 'Gln',
	'cac': 'His',
	'cag': 'Gln',
	'cat': 'His',
	'cca': 'Pro',
	'ccc': 'Pro',
	'ccg': 'Pro',
	'cct': 'Pro',
	'cga': 'Arg',
	'cgc': 'Arg',
	'cgg': 'Arg',
	'cgt': 'Arg',
	'cta': 'Leu',
	'ctc': 'Leu',
	'ctg': 'Leu',
	'ctt': 'Leu',
	'gaa': 'Glu',
	'gac': 'Asp',
	'gag': 'Glu',
	'gat': 'Asp',
	'gca': 'Ala',
	'gcc': 'Ala',
	'gcg': 'Ala',
	'gct': 'Ala',
	'gga': 'Gly',
	'ggc': 'Gly',
	'ggg': 'Gly',
	'ggt': 'Gly',
	'gta': 'Val',
	'gtc': 'Val',
	'gtg': 'Val',
	'gtt': 'Val',
	'taa': 'STP',
	'tac': 'Tyr',
	'tag': 'STP',
	'tat': 'Tyr',
	'tca': 'Ser',
	'tcc': 'Ser',
	'tcg': 'Ser',
	'tct': 'Ser',
	'tga': 'STP',
	'tgc': 'Cys',
	'tgg': 'Trp',
	'tgt': 'Cys',
	'tta': 'Leu',
	'ttc': 'Phe',
	'ttg': 'Leu',
	'ttt': 'Phe'
} ;

function reverse_complement( sequence ) {
	sequence.reverse() ;
	return sequence.map( elt => reverse_complement_map[elt] ) ;
} ;

function compute_amino_acid_sequence( CDS, reference, strand ) {
	// Could be passed this from MSAView, but let's compute it here
	let mapPhysicalToSequenceCoord = new d3.scaleLinear()
		.domain( [ reference.coordinateRange.start, reference.coordinateRange.end ] )
		.range( [ 0, reference.sequence.length - 1 ] ) ;

	let sequence = reference.sequence ;

	let result = [] ;
	let coding_length = 0 ;
	for( let i = 0; i < CDS.length; ++i ) {
		coding_length += CDS[i].end - CDS[i].start + 1 ;
	}
	if( coding_length % 3 == 0 ) {
		let coding_sequence = new Uint8Array( coding_length ) ;
		let breaks = [] ;
		let where = 0 ;
		for( let i = 0; i < CDS.length; ++i ) {
			let length = CDS[i].end - CDS[i].start + 1 ;
			let a = mapPhysicalToSequenceCoord( CDS[i].start ) ;
			let b = mapPhysicalToSequenceCoord( CDS[i].end + 1 ) ; // convert to open interval
			coding_sequence.subarray( where, where + length ).set( sequence.slice( a, b ) ) ;
			where = where + length ;
		}

		let sequence_position = 0 ;
		let oddeven = 0 ;
		let count = 0 ;
		let aa = '' ;
		let consumed = 3 ;
		let decoder = new TextDecoder() ;
		for( let i = 0; i < CDS.length; ++i ) {
			let physical_position = CDS[i].start + ((3-consumed) % 3) ;
			if( consumed < 3 ) {
				let end = Math.min( physical_position - 1, CDS[i].end ) ;
				result.push({
					'start': CDS[i].start,
					'end': end,
					'aa': aa,
					'count': count,
					'oddeven': oddeven % 2
				}) ;
				consumed += (end - CDS[i].start) ;
			}

			for( ; physical_position <= CDS[i].end; sequence_position += 3, physical_position += 3 ) {
				let codon = coding_sequence.slice( sequence_position, sequence_position+3 ) ;
				if( strand == '-' ) {
					codon = reverse_complement(codon) ;
				}
				aa = aa_table[decoder.decode(codon)] ;
				let end = Math.min( physical_position+2, CDS[i].end ) ;
				result.push({
					'start': physical_position,
					'end': end,
					'aa': aa,
					'count': (++count),
					'oddeven': (++oddeven) % 2
				}) ;
				consumed = (end + 1) - physical_position ;
			}
		}
	}
	return result ;
} ;

// data be an array of objects representing genes, including
// ID, feature, chromosome, start, and end columns.
function GeneView(
	data,
	reference,
	highlight = []
) {
	this.data = data ;
	this.reference = reference ;

	this.region = function( region ) {
		return {
			chromosome: region.chromosome,
			start: Math.min( region.start, region.end ),
			end: Math.max( region.start, region.end )
		} ;
	}( reference.coordinateRange ) ;
	this.highlight = highlight ;

	this.genes = function( genes, region, highlight ) {
		// decompose genes data by exons and transcripts.
		console.log( "getRegionalGenes", genes, region ) ;
		
		let result = genes.filter(
			elt => (
				[ 'gene', 'transcript', 'mRNA', 'tRNA', 'rRNA', 'snoRNA', 'ncRNA', 'snRNA' ].indexOf( elt.feature ) != -1
				&& elt.chromosome == region.chromosome
				&& elt.end >= region.start
				&& elt.start <= region.end
			)
		) ;
		result.sort( function( a, b ) {
			var c = a.start - b.start ;
			if( c == 0 ) {
				c = a.end - b.end ;
			}
			return c ;
		} ) ;


		for( var j = 0; j < result.length; ++j ) {
			let gene = result[j] ;
			if( gene.strand == "-" ) {
				gene.txStart = gene.end ;
				gene.txEnd = gene.start ;
			} else {
				gene.txStart = gene.start ;
				gene.txEnd = gene.end ;
			}
			
			let exonData = genes.filter(
				elt => (elt.feature == 'exon' || elt.feature == 'CDS')
				&& elt.parent == gene.ID
				&& elt.end >= region.start
				&& elt.start <= region.end
			).map( elt => Object.assign( elt, { level: gene.level } )) ;

			result[j].exons = exonData ;
			result[j].amino_acids = compute_amino_acid_sequence(
				exonData.filter( elt => elt.feature == 'CDS' ),
				reference,
				gene.strand
			) ;

			result[j].highlight = ( highlight.indexOf( gene.symbol ) != -1 ) ? 1 : 0 ;
		}
		return( result ) ;
	}( this.data, this.region, highlight ) ;

	let layoutIntervals = function( genes, spacer = { start: 10, end: 20000 } ) {
		// genes should be an array of objects with start and end properties
		// This function modifies genes in-place to add 'level' property suitable
		// for vertically separating genes on a plot.
		if( genes.length == 0 ) {
			return genes ;
		}
		let level_endpoints = [genes[0].end + spacer.end];
		genes[0].level = level_endpoints.length ;
		for( var i = 1; i < genes.length; ++i ) {
			let gene = genes[i] ;
			let start_with_spacer = gene.start - spacer.start ;
			let end_with_spacer = gene.end + spacer.end ;
			let assigned = false ;
			for( var l = 0; l < level_endpoints.length; ++l ) {
				if( start_with_spacer > level_endpoints[l] ) {
					genes[i].level = l+1 ;
					level_endpoints[l] = end_with_spacer ;
					assigned = true ;
					break ;
				}
			}
	
			// Otherwise add a new level
			if( !assigned ) {
				level_endpoints.push( end_with_spacer ) ;
				genes[i].level = level_endpoints.length ;
			}
		
		}
		for( var i = 0; i < genes.length; ++i ) {
			for( var j = 0; j < genes[i].exons.length; ++j ) {
				genes[i].exons[j].level = genes[i].level ;
			}
			for( var j = 0; j < genes[i].amino_acids.length; ++j ) {
				genes[i].amino_acids[j].level = genes[i].level ;
			}
		}

		// Return the number of levels
		return( level_endpoints.length ) ;
	}
	

	let direction = "horizontal" ;
	this.numberOfLevels = layoutIntervals(
		this.genes,
		{ start: (direction == "vertical" ? 5000 : 1000), end: (direction == "vertical" ? 10 : 1000) }
	) ;
	this.count = this.genes.length ;
	console.log( "GeneView()", this.data, this.region, this.genes ) ;
	return this ;
}

// get hierarchical structure representing regional genes

// elt is a D3 selection
// genes is an array of objects with keys: chromosome, feature,. start, end, ID, symbol, strand, name, txStart and txEnd
// scales is an object containing 'position' and 'level' keys, each a d3 scale.
GeneView.prototype.draw = function(
	elt,
	scales,
	direction = "horizontal",
	aesthetic = {
		"colour": {
			"highlight": "#FF4444",
			"text": "#eeeeee",
			"line": "#eeeeee"
		}
	}
) {
	let aes = aesthetic ;
	let e = {
		"exon": 0.125,
		"CDS": 0.25
	} ;
	let h = 0.2 ;
	let H = 0.4 ;
	let a = {
		'-': -1,
		'+': 1
	} ;
	let t = 4 ;
	let textTransform = ( direction == "vertical" ) ? "rotate(-90)" : "" ;
	let axes = {} ;
	let baseWidth = Math.abs( scales.position(1) - scales.position(0)) ;
	let orientation = Math.sign( scales.position(1) - scales.position(0)) ;

	if( direction == "horizontal" ) {
		axes = {
			"x": "x",
			"y": "y",
			"x1": "x1",
			"x2": "x2",
			"y1": "y1",
			"y2": "y2",
			"width": "width",
			"height": "height"
		} ;
	} else if( direction == "vertical" ) {
		axes = {
			"x": "y",
			"y": "x",
			"x1": "y1",
			"x2": "y2",
			"y1": "x1",
			"y2": "x2",
			"width": "height",
			"height": "width"
		} ;
	} else {
		//console.log( "GeneView.draw()", "Unrecognised direction \"" + direction + "\", quitting." )
		return ;
	}
	//console.log( "GeneView.draw()", this.genes, orientation ) ;
	let min = Math.min ;
	let max = Math.max ;
	let abs = Math.abs ;
	let enter = elt.selectAll( 'g.gene' )
		.data( this.genes.filter( elt => elt.feature == "mRNA" || elt.feature == "transcript" ) )
		.enter()
		.append( 'g' )
		.attr( 'class', 'gene' ) ;
	let self = this ;
	let region = self.region ;
	enter
		.append( 'line' )
		.attr( 'class', 'mid' ) ;
	enter
		.append( 'line' )
		.attr( 'class', 'bar_s' ) ;
	enter
		.append( 'line' )
		.attr( 'class', 'bar_e' ) ;
	enter
		.append( 'line' )
		.attr( 'class', 'dash_s' ) ;
	enter
		.append( 'line' )
		.attr( 'class', 'dash_e' ) ;
	enter
		.append( 'line' )
		.attr( 'class', 'arrow_body' ) ;
	enter
		.append( 'line' )
		.attr( 'class', 'arrow_lower' ) ;
	enter
		.append( 'line' )
		.attr( 'class', 'arrow_upper' ) ;
	enter
		.append( 'text' )
		.attr( 'class', 'symbol' ) ;

	let transcripts = elt.selectAll( 'g.gene' ) ;
	transcripts
		.selectAll( 'line.mid' )
		.attr( axes.x1, elt => scales.position( Math.max( elt.start, self.region.start )) )
		.attr( axes.x2, elt => scales.position( Math.min( elt.end, self.region.end )) )
		.attr( axes.y1, elt => scales.level( elt.level ))
		.attr( axes.y2, elt => scales.level( elt.level ))
		.attr( "stroke", "black" )
	;
	
	let exons = transcripts.selectAll( 'rect.exon' )
		.data( elt => elt.exons ) ;
	exons
		.enter()
		.append( 'rect' )
		.attr( 'class', 'exon' ) ;
	transcripts.selectAll( 'rect.exon' )
		// SVG widths / heights must be positive.
		// The following formulation makes sure we plot rectangles
		// in the right direction even when the scale is reversed.
		.attr( axes.x, elt => min( scales.position( elt.start - 0.5 ), scales.position( elt.end - 0.5 )) )
		.attr( axes.width, elt => abs( scales.position( elt.end + 0.5 ) - scales.position( elt.start - 0.5 )) )
		.attr( axes.y, elt => min( scales.level( elt.level + e[elt.feature] ), scales.level( elt.level - e[elt.feature] ) ))
		.attr( axes.height, elt => abs( scales.level( -2 * e[elt.feature] ) - scales.level(0) ))
		//.attr( 'stroke', 'black' )
		.attr( 'fill', 'grey' )
	;

	let aa_sequence = transcripts.selectAll( 'g.aa' )
		.data( elt => elt.amino_acids ) ;
	console.log( "AA", this.genes.filter( elt => elt.feature == 'transcript' )[0].amino_acids ) ;
	let aa_enter = aa_sequence
		.enter()
		.append( 'g' )
		.attr( 'class', 'aa' ) ;
	aa_enter.append( 'rect' ) ;
	aa_enter.append( 'text' ) ;

	let interpolator = 0 ;
	if( baseWidth > 2 ) {
		interpolator = Math.min( (baseWidth-2) / 5.0, 1.0 ) ;
	}

	transcripts.selectAll( 'g.aa rect' )
		.attr( axes.x, elt => min( scales.position( elt.start - 0.5 ), scales.position( elt.end - 0.5 )) )
		.attr( axes.width, elt => abs( scales.position( elt.end + 0.5 ) - scales.position( elt.start - 0.5 )) )
		.attr( axes.y, elt => min( scales.level( elt.level + e['CDS'] ), scales.level( elt.level - e['CDS'] ) ))
		.attr( axes.height, elt => abs( scales.level( -2 * e['CDS'] ) - scales.level(0) ))
		//.attr( 'stroke', 'black' )
		//.attr( 'fill', elt => `rgba( 128 + 80*${elt.oddeven}, 128 + 80*${elt.oddeven}, 128 + 80*${elt.oddeven}, ${interpolator} )` )
		.attr( 'fill', elt => ( elt.oddeven == 1 ) ? `rgba( 108, 108, 108, ${interpolator} )` : `rgb( 148, 148, 148, ${interpolator} )` )
	;

	let interpolator2 = 0 ;
	if( baseWidth > 15 ) {
		interpolator2 = Math.min( (baseWidth-15) / 30.0, 1.0 ) ;
	}
	transcripts.selectAll( 'g.aa text' )
		// At the time of writing I don't understand why I need to subtract 1 here!
		// But it puts the text in the middle.
		.attr( axes.x, elt => (scales.position( (elt.start + elt.end )/2 )) )
		.attr( axes.y, elt => scales.level( elt.level ))
		.attr( 'text-anchor', 'middle' )
		.attr( 'alignment-baseline', 'middle' )
		.attr( 'font-size', '8pt' )
		.attr( 'font-family', 'Helvetica' )
		.attr( 'stroke', '#EEEEEE' )
		.attr( 'stroke-opacity', interpolator2 )
		.attr( 'fill-opacity', interpolator2 )
		.text( elt => ( elt.aa + "    " + elt.count )) ;

	function isVisible( x ) {
		return x >= region.start && x <= region.end ;
	}
	
	let offset = orientation*(baseWidth/2) ;
	
	// 0.5 added to start/end to make gene encompass the whole base
	// start bar
	transcripts
		.selectAll( 'line.bar_s' )
		.filter( elt => isVisible(elt.txStart))
		.attr( axes.x1, elt => scales.position( elt.txStart - 0.5 ))
		.attr( axes.x2, elt => scales.position( elt.txStart - 0.5 ))
		.attr( axes.y1, elt => scales.level( elt.level - h ))
		.attr( axes.y2, elt => scales.level( elt.level + H ) - 0.5 )
		.attr( 'stroke', 'black' )
	;
	// end bar
	transcripts
		.selectAll( 'line.bar_e' )
		.filter( elt => isVisible( elt.txEnd ))
		.attr( axes.x1, elt => scales.position( elt.txEnd + 0.5 ))
		.attr( axes.x2, elt => scales.position( elt.txEnd + 0.5 ))
		.attr( axes.y1, elt => scales.level( elt.level - h ))
		.attr( axes.y2, elt => scales.level( elt.level + h ))
		.attr( 'stroke', 'black' )
	;
	// start dash
	transcripts
		.selectAll( 'line.dash_s' )
		.filter( elt => !isVisible(elt.start))
		.attr( axes.x1, elt => scales.position( region.start - 0.5 ) - 10 )
		.attr( axes.x2, elt => scales.position( region.start - 0.5 ))
		.attr( axes.y1, elt => scales.level( elt.level ))
		.attr( axes.y2, elt => scales.level( elt.level ))
		.attr( 'stroke-dasharray', '2' )
		.attr( 'stroke', 'black' )
	;
	// end dash
	transcripts
		.selectAll( 'line.dash_e' )
		.filter( elt => !isVisible( elt.end ))
		.attr( axes.x1, elt => scales.position( region.end + 0.5 ) )
		.attr( axes.x2, elt => scales.position( region.end + 0.5 ) + orientation * 10 )
		.attr( axes.y1, elt => scales.level( elt.level ))
		.attr( axes.y2, elt => scales.level( elt.level ))
		.attr( 'stroke-dasharray', '2' )
		.attr( 'stroke', 'black' )
	;

	// arrow
	transcripts
		.selectAll( 'line.arrow_body' )
		.attr( axes.x1, elt => scales.position( elt.txStart - 0.5 ))
		.attr( axes.x2, elt => scales.position( elt.txStart - 0.5 ) + orientation * a[elt.strand] * 4 )
		.attr( axes.y1, elt => scales.level( elt.level + H ))
		.attr( axes.y2, elt => scales.level( elt.level + H ))
		.attr( 'stroke', 'black' )
	;
	transcripts
		.selectAll( 'line.arrow_lower' )
		.attr( axes.x1, elt => scales.position( elt.txStart - 0.5 ) + orientation * a[elt.strand] * 4 )
		.attr( axes.x2, elt => scales.position( elt.txStart - 0.5 ) + orientation * a[elt.strand] * 2 )
		.attr( axes.y1, elt => scales.level( elt.level + H ))
		.attr( axes.y2, elt => scales.level( elt.level + H ) - 2 )
		.attr( 'stroke', 'black' )
	;
	transcripts
		.selectAll( 'line.arrow_upper' )
		.attr( axes.x1, elt => scales.position( elt.txStart - 0.5 ) + orientation * a[elt.strand] * 4 )
		.attr( axes.x2, elt => scales.position( elt.txStart - 0.5 ) + orientation * a[elt.strand] * 2 )
		.attr( axes.y1, elt => scales.level( elt.level + H ))
		.attr( axes.y2, elt => scales.level( elt.level + H ) + 2 )
		.attr( 'stroke', 'black' )
	;

	if( Math.abs( scales.position(1000) - scales.position(0) ) < 15 ) {
		transcripts
		.selectAll( 'text.symbol' )
		.text( "") ;
	}
	else {
		let font_size = '5pt' ;
		if( Math.abs( scales.position(1000) - scales.position(0) ) > 25 ) {
			font_size = '6pt' ;
		} 
		if( Math.abs( scales.position(1000) - scales.position(0) ) > 35 ) {
			font_size = '7pt' ;
		}
		transcripts
			.selectAll( 'text.symbol' )
			.attr( 'transform', function( elt ) {
				if( direction == "horizontal" ) {
					return "translate(" + (scales.position( elt.end ) + offset + orientation * t) + ", " + scales.level( elt.level ) + ")" ;
				} else if( direction == "vertical" ) {
					return "translate(" + scales.level( elt.level ) + ", " + (scales.position( elt.start ) + t) + ") rotate(90)" ;
				}
			})
	//		.attr( axes.x, elt => scales.position( elt.end ) + t )
	//		.attr( axes.y, elt => scales.level( elt.level ))
			.attr( 'alignment-baseline', 'central' )
			.attr( 'text-anchor', ( orientation == 1 ) ? 'start' : 'end' )
			.attr( 'font-size', font_size )
			.attr( 'font-weight', elt => ( elt.highlight == 1 ) ? 'bold' : 'normal' )
			.attr( 'fill', elt => ( elt.highlight == 1 ) ? aes.colour.highlight : aes.colour.text )
			.attr( 'font-style', 'italic' )
			.text( elt => elt.display ? elt.display : elt.ID )
		;
	}
}
