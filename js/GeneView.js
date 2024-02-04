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

let iupac = {
	'A': { 'three': 'Ala', 'name': 'Alanine' },
	'C': { 'three': 'Cys', 'name': 'Cysteine' },
	'D': { 'three': 'Asp', 'name': 'Aspartic Acid' },
	'E': { 'three': 'Glu', 'name': 'Glutamic Acid' },
	'F': { 'three': 'Phe', 'name': 'Phenylalanine' },
	'G': { 'three': 'Gly', 'name': 'Glycine' },
	'H': { 'three': 'His', 'name': 'Histidine' },
	'I': { 'three': 'Ile', 'name': 'Isoleucine' },
	'K': { 'three': 'Lys', 'name': 'Lysine' },
	'L': { 'three': 'Leu', 'name': 'Leucine' },
	'M': { 'three': 'Met', 'name': 'Methionine' },
	'N': { 'three': 'Asn', 'name': 'Asparagine' },
	'P': { 'three': 'Pro', 'name': 'Proline' },
	'Q': { 'three': 'Gln', 'name': 'Glutamine' },
	'R': { 'three': 'Arg', 'name': 'Arginine' },
	'S': { 'three': 'Ser', 'name': 'Serine' },
	'T': { 'three': 'Thr', 'name': 'Threonine' },
	'V': { 'three': 'Val', 'name': 'Valine' },
	'W': { 'three': 'Trp', 'name': 'Tryptophan' },
	'Y': { 'three': 'Tyr', 'name': 'Tyrosine}' }
} ;

let aa_table = {
	'aaa': 'K', 'aac': 'N', 'aag': 'K', 'aat': 'N', 'aca': 'T',
	'acc': 'T', 'acg': 'T', 'act': 'T', 'aga': 'R', 'agc': 'S',
	'agg': 'R', 'agt': 'S', 'ata': 'I', 'atc': 'I', 'atg': 'M',
	'att': 'I', 'caa': 'Q', 'cac': 'H', 'cag': 'Q', 'cat': 'H',
	'cca': 'P', 'ccc': 'P', 'ccg': 'P', 'cct': 'P', 'cga': 'R',
	'cgc': 'R', 'cgg': 'R', 'cgt': 'R', 'cta': 'L', 'ctc': 'L',
	'ctg': 'L', 'ctt': 'L', 'gaa': 'Q', 'gac': 'D', 'gag': 'Q',
	'gat': 'D', 'gca': 'A', 'gcc': 'A', 'gcg': 'A', 'gct': 'A',
	'gga': 'G', 'ggc': 'G', 'ggg': 'G', 'ggt': 'G', 'gta': 'V',
	'gtc': 'V', 'gtg': 'V', 'gtt': 'V', 'taa': 'STOP', 'tac': 'Y',
	'tag': 'STOP', 'tat': 'Y', 'tca': 'S', 'tcc': 'S', 'tcg': 'S',
	'tct': 'S', 'tga': 'STOP', 'tgc': 'C', 'tgg': 'W', 'tgt': 'C',
	'tta': 'L', 'ttc': 'F', 'ttg': 'L', 'ttt': 'F'
} ;

function reverse_complement( sequence ) {
	sequence.reverse() ;
	return sequence.map( elt => reverse_complement_map[elt] ) ;
} ;

function compute_amino_acid_sequence( CDS, reference, strand ) {
	// Could be passed this from MSAView, but let's compute it here
	let physicalToSequenceScale = new d3.scaleLinear()
		.domain( [ reference.coordinateRange.start, reference.coordinateRange.end ] )
		.range( [ 0, reference.sequence.length - 1 ] ) ;
	let mapPhysicalToSequenceCoord = function(x) {
		return Math.round( physicalToSequenceScale(x)) ;
	}
	let sequence = reference.sequence ;

	let result = [] ;
	// compute total length of cds
	let coding_length = 0 ;
	for( let i = 0; i < CDS.length; ++i ) {
		coding_length += CDS[i].end - CDS[i].start + 1 ;
	}
	if( 1 ) {
		let coding_sequence = new Uint8Array( coding_length ) ;
		let where = 0 ;
		for( let i = 0; i < CDS.length; ++i ) {
			let length = CDS[i].end - CDS[i].start + 1 ;
			let a = mapPhysicalToSequenceCoord( CDS[i].start ) ;
			let b = mapPhysicalToSequenceCoord( CDS[i].end + 1 ) ; // convert to open interval
			coding_sequence.subarray( where, where + length ).set( sequence.slice( a, b ) ) ;
			where = where + length ;
		}

		// The above coding sequence is in position order.
		// We assume that the aa sequences starts at the first base of the CDS.
		// For reverse orientation genes this means the last base of the above sequence.
		// This adds a complication because we process the sequence in position order.
		// We keep track of the index of the AA in the coding sequence, and the number of
		// bases 'consumed' so far so we can handle AAs overlapping CDS boundaries.
		let oddeven = 0 ;
		let orientation = (strand == '+') ? 1 : -1 ;
		let aa_index = (orientation == 1) ? 0 : (Math.floor( coding_sequence.length/3 ))+1 ;
		let offset = (orientation == 1) ? 0 : coding_length % 3 ;
		let aa = '?' ;
		let index_in_sequence = 0 ;

		let decoder = new TextDecoder() ;
		for( let i = 0; i < CDS.length; ++i ) {
			let consumed = Math.abs(index_in_sequence + (3-offset)) % 3 ;
			let physical_position = CDS[i].start ;
			// console.log( "Stt of CDS: i = ", i, "CDS: ", CDS[i].ID, physical_position, index_in_sequence, consumed, "offset:", offset ) ;
			if( consumed != 0 ) {
				console.log( "aa", aa, "consumed", consumed, physical_position ) ;
				// We are not at an AA boundary.
				// Handle the remainder of the preceding AA.
				// In an extreme case the AA could span three CDS records, I suppose
				// so the code must handle this.
				let start = CDS[i].start ;
				let end = Math.min( start + 3-consumed - 1, CDS[i].end ) ;
				let length = end - start + 1 ;
				result.push({
					'start': start,
					'end': end,
					'aa': aa,
					'count': aa_index,
					'oddeven': oddeven % 2
				}) ;
				index_in_sequence += length ;
				physical_position += length ;
			}

			// in this loop, index_in_sequence always points to the
			// start of an AA.
			while( physical_position <= CDS[i].end ) {
				aa_index += orientation ;
				let codon = coding_sequence.slice(
					index_in_sequence,
					Math.min(index_in_sequence+3,coding_sequence.length)
				) ;
				if( strand == '-' ) {
					codon = reverse_complement(codon) ;
				}
				aa = (codon.length == 3) ? aa_table[decoder.decode(codon)] : "?" ;
				let end = Math.min( physical_position+2, CDS[i].end ) ;
				let length = end - physical_position + 1 ;
				result.push({
					'start': physical_position,
					'end': end,
					'aa': aa,
					'count': aa_index,
					'oddeven': (++oddeven) % 2
				}) ;

				index_in_sequence += length ;
				physical_position += length ;
			}
			// console.log( "End of CDS: i = ", i, "CDS: ", CDS[i].ID, physical_position, index_in_sequence, consumed ) ;
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
		let sortFn = function( a, b ) {
			var c = a.start - b.start ;
			if( c == 0 ) {
				c = a.end - b.end ;
			}
			return c ;
		} ;
		result.sort( sortFn ) ;

		for( var j = 0; j < result.length; ++j ) {
			let gene = result[j] ;
			if( gene.strand == "-" ) {
				gene.txStart = gene.end ;
				gene.txEnd = gene.start ;
				gene.orientation = -1 ;
			} else {
				gene.txStart = gene.start ;
				gene.txEnd = gene.end ;
				gene.orientation = 1 ;
			}
			
			let exonData = genes.filter(
				elt => (elt.feature == 'exon' || elt.feature == 'CDS')
				&& elt.parent == gene.ID
				&& elt.end >= region.start
				&& elt.start <= region.end
			).map( elt => Object.assign( elt, { level: gene.level } )) ;
			exonData.sort( sortFn ) ;

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

	let self = this ;
	let region = self.region ;
	let min = Math.min ;
	let max = Math.max ;
	let abs = Math.abs ;
	let domain = scales.position.domain() ;
	let domain_filter = function( elt ) {
		return (elt.start <= domain[1] ) && (elt.end >= domain[0] ) ;
	} ;
	let visible_genes = this.genes.filter( elt => (
		(elt.feature == "mRNA" || elt.feature == "transcript" )
		&& domain_filter(elt)
	)) ;

	let transcripts = elt.selectAll( 'g.gene' )
		.data( visible_genes, elt => elt.ID )
		.join(
			function(enter) {
				let g = enter.append( 'g' )
					.attr( 'class', 'gene' ) ;
				g.append( 'line' )
					.attr( 'class', 'mid' ) ;
				g.append( 'line' )
					.attr( 'class', 'bar_s' ) ;
				g.append( 'line' )
					.attr( 'class', 'bar_e' ) ;
				g.append( 'line' )
					.attr( 'class', 'dash_s' ) ;
				g.append( 'line' )
					.attr( 'class', 'dash_e' ) ;
				g.append( 'line' )
					.attr( 'class', 'arrow_body' ) ;
				g.append( 'line' )
					.attr( 'class', 'arrow_lower' ) ;
				g.append( 'line' )
					.attr( 'class', 'arrow_upper' ) ;
				g.append( 'text' )
					.attr( 'class', 'symbol' ) ;
				g.append( 'g' )
					.attr( 'class', 'exons' ) ;
				g.append( 'g' )
					.attr( 'class', 'aas' ) ;
				return g ;
			},
			function( update ) {
				return update.attr( 'class', 'gene' ) ;
			},
			function(exit) {
				return exit.remove() ;
			}
		) ;

	transcripts = elt.selectAll( 'g.gene' ) ;
	let visualexons = transcripts.selectAll( 'g.exons' )
		.data( elt => [ elt.exons ] ) ;
	visualexons.selectAll( 'rect.exon' )
		.data( elt => elt.filter( domain_filter ), elt => elt.ID )
		.join(
			function(enter) {
				return enter.append( 'rect' )
					.attr( 'class', 'exon' ) ;
			},
			function(update ) {
				return update.attr( 'class', 'exon' ) ;
			},
			function( exit ) {
				return exit.remove() ;
			}	
		) ;

	let visualaas = transcripts.selectAll( 'g.aas' )
		.data( elt => [ elt.amino_acids ] ) ;
	visualaas.selectAll( 'g.aa' )
		.data( elt => ((baseWidth > 1) ? elt : [] ), elt => elt.count )
		.join(
			function( enter ) {
				let g = enter.append('g')
					.attr( 'class', 'aa' )
				;
				g.append( 'rect' ).attr( 'class', elt => elt.count ) ; ;
				g.append( 'text' ).attr( 'class', 'aa' ) ;
				g.append( 'text' ).attr( 'class', 'count' ) ;
				return g ;
			},
			function(update ) {
				return update.attr( 'class', 'aa' ) ;
			},
			function( exit ) {
				return exit.remove() ;
			}
		) ;

	transcripts
		.selectAll( 'line.mid' )
		.attr( axes.x1, elt => scales.position( Math.max( elt.start, self.region.start ) - 0.5 ) )
		.attr( axes.x2, elt => scales.position( Math.min( elt.end, self.region.end ) + 0.5 ) )
		.attr( axes.y1, elt => scales.level( elt.level ))
		.attr( axes.y2, elt => scales.level( elt.level ))
		.attr( "stroke", "black" )
	;

	let exons = transcripts.selectAll( 'rect.exon' ) ;
	//exons.exit().remove() ;
	exons
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

	let aa_sequence = transcripts.selectAll( 'g.aa' ) ;

	let interpolator1 = 0 ;
//	if( (3*baseWidth) >= 0.1 ) {
		interpolator1 = Math.min( ((baseWidth)-0.1) / 10.0, 1.0 ) ;
//	}
	let interpolator2 = 0 ;
	if( baseWidth > 8 ) {
		interpolator2 = Math.min( (baseWidth-8) / 10.0, 1.0 ) ;
	}

	let interpolator3 = 0 ;
	if( baseWidth > 20 ) {
		interpolator3 = Math.min( (baseWidth-20) / 10.0, 1.0 ) ;
	}

	transcripts.selectAll( 'g.aa rect' )
		.attr( axes.x, elt => min( scales.position( elt.start - 0.5 ), scales.position( elt.end - 0.5 )) )
		.attr( axes.width, elt => abs( scales.position( elt.end + 0.5 ) - scales.position( elt.start - 0.5 )) )
		.attr( axes.y, elt => min( scales.level( elt.level + e['CDS'] ), scales.level( elt.level - e['CDS'] ) ))
		.attr( axes.height, elt => abs( scales.level( -2 * e['CDS'] ) - scales.level(0) ))
		//.attr( 'stroke', 'black' )
		//.attr( 'fill', elt => `rgba( 128 + 80*${elt.oddeven}, 128 + 80*${elt.oddeven}, 128 + 80*${elt.oddeven}, ${interpolator} )` )
		.attr( 'fill', function(elt) {
			if( elt.aa == 'STOP' ) {
				return `rgba( 148, 20, 3, ${interpolator1} )` ;
			} else if( elt.aa == "?" ) {
				return `rgba( 188, 20, 180, ${interpolator1} )` ;
			} else if( elt.oddeven == 1 ) {
				return `rgba( 108, 128, 148, ${interpolator1} )` ;
				//return `rgba( 118, 118, 20, ${interpolator} )` ;
			} else {
				return `rgb( 128, 128, 108, ${interpolator1} )` ;
				//return `rgba( 118, 20, 118, ${interpolator} )` ;
			}
		})
	;

	transcripts.selectAll( 'g.aa text.aa' )
		.attr( axes.x, elt => (scales.position( (elt.start + elt.end )/2 )) )
		.attr( axes.y, elt => scales.level( elt.level ))
		.attr( 'text-anchor', 'middle' )
		.attr( 'alignment-baseline', 'middle' )
		.attr( 'font-size', '8pt' )
		.attr( 'font-family', 'Palatino' )
		.attr( 'font-weight', 'bold' )
		.attr( 'stroke', '#CCCCCC' )
		.attr( 'stroke-opacity', interpolator2 )
		.attr( 'fill-opacity', interpolator2 )
		.text( elt => ( elt.aa == "STOP" ? "X" : elt.aa )) ;

	transcripts.selectAll( 'g.aa text.count' )
		.attr( axes.x, elt => (scales.position( ( 0.5 * elt.start + 0.5 * elt.end ) + 0.33 )) )
		.attr( axes.y, elt => scales.level( elt.level ))
		.attr( 'text-anchor', 'middle' )
		.attr( 'alignment-baseline', 'middle' )
		.attr( 'font-size', '7pt' )
		.attr( 'font-family', 'Palatino' )
		.attr( 'stroke', '#ABABAB' )
		.attr( 'stroke-opacity', interpolator3 )
		.attr( 'fill-opacity', interpolator3 )
		.text( elt => "(" + elt.count + ")" ) ;

	function isVisible( x ) {
		return x >= region.start && x <= region.end ;
	}
	
	let offset = orientation*(baseWidth/2) ;
	
	// 0.5 added to start/end to make gene encompass the whole base
	// start bar
	transcripts
		.selectAll( 'line.bar_s' )
		.filter( elt => isVisible(elt.txStart))
		.attr( axes.x1, elt => scales.position( elt.txStart - elt.orientation * 0.5 ))
		.attr( axes.x2, elt => scales.position( elt.txStart - elt.orientation * 0.5 ))
		.attr( axes.y1, elt => scales.level( elt.level - h ))
		.attr( axes.y2, elt => scales.level( elt.level + H ) - 0.5 )
		.attr( 'stroke', 'black' )
	;
	// end bar
	transcripts
		.selectAll( 'line.bar_e' )
		.filter( elt => isVisible( elt.txEnd ))
		.attr( axes.x1, elt => scales.position( elt.txEnd + elt.orientation * 0.5 ))
		.attr( axes.x2, elt => scales.position( elt.txEnd + elt.orientation * 0.5 ))
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
		.attr( axes.x1, elt => scales.position( elt.txStart - elt.orientation * 0.5 ))
		.attr( axes.x2, elt => scales.position( elt.txStart - elt.orientation * 0.5 ) + orientation * a[elt.strand] * 4 )
		.attr( axes.y1, elt => scales.level( elt.level + H ))
		.attr( axes.y2, elt => scales.level( elt.level + H ))
		.attr( 'stroke', 'black' )
	;
	transcripts
		.selectAll( 'line.arrow_lower' )
		.attr( axes.x1, elt => scales.position( elt.txStart - elt.orientation * 0.5 ) + orientation * a[elt.strand] * 4 )
		.attr( axes.x2, elt => scales.position( elt.txStart - elt.orientation * 0.5 ) + orientation * a[elt.strand] * 2 )
		.attr( axes.y1, elt => scales.level( elt.level + H ))
		.attr( axes.y2, elt => scales.level( elt.level + H ) - 2 )
		.attr( 'stroke', 'black' )
	;
	transcripts
		.selectAll( 'line.arrow_upper' )
		.attr( axes.x1, elt => scales.position( elt.txStart - elt.orientation * 0.5 ) + orientation * a[elt.strand] * 4 )
		.attr( axes.x2, elt => scales.position( elt.txStart - elt.orientation * 0.5 ) + orientation * a[elt.strand] * 2 )
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
