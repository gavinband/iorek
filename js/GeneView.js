"use strict";

// data be an array of objects representing genes, including
// ID, feature, chromosome, start, and end columns.
function GeneView(
	data,
	region,
	highlight = []
) {
	this.data = data ;
	this.region = region ;
	this.highlight = highlight ;

	let getRegionalGenes = function( genes, region, highlight ) {
		// decompose genes data by exons and transcripts.
		console.log( "getRegionalGenes", genes, region ) ;
		let result = genes.filter(
			elt => (
				[ 'gene', 'mRNA', 'tRNA', 'rRNA', 'snoRNA', 'ncRNA', 'snRNA' ].indexOf( elt.feature ) != -1
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
				elt => elt.feature == 'exon'
				&& elt.parent == gene.ID
				&& elt.end >= region.start
				&& elt.start <= region.end
			).map( elt => Object.assign( elt, { level: gene.level } )) ;
			result[j].exons = exonData ;
			result[j].highlight = ( highlight.indexOf( gene.symbol ) != -1 ) ? 1 : 0 ;
		}
		return( result ) ;
	}

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
		}

		// Return the number of levels
		return( level_endpoints.length ) ;
	}
	
	this.genes = getRegionalGenes( this.data, region, highlight ) ;

	let direction = "horizontal" ;
	this.numberOfLevels = layoutIntervals(
		this.genes,
		{ start: (direction == "vertical" ? 2000 : 10), end: (direction == "vertical" ? 10 : 1000) }
	) ;
	console.log( "GeneView()", this.data, region, this.genes ) ;
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
	let e = 0.2 ;
	let h = 0.2 ;
	let H = 0.4 ;
	let a = {
		'-': -1,
		'+': 1
	} ;
	let t = 4 ;
	let orient = ( direction == "vertical" ) ? -1 : 1 ; // handles upside-downyness of svg canvas coords
	let textTransform = ( direction == "vertical" ) ? "rotate(-90)" : "" ;
	let axes = {} ;
	
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
		console.log( "drawGenes()", "Unrecognised direction \"" + direction + "\", quitting." )
		return ;
	}
	let min = Math.min ;
	let max = Math.max ;
	let abs = Math.abs ;
	let enter = elt.selectAll( 'g.gene' )
		.data( this.genes.filter( elt => elt.feature == "mRNA" ) )
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
		.attr( axes.x1, elt => scales.position( Math.max( elt.start, self.region.start )))
		.attr( axes.x2, elt => scales.position( Math.min( elt.end, self.region.end )))
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
		// in the right direction even when the scale is upside-down.
		.attr( axes.x, elt => min( scales.position( elt.start ), scales.position( elt.end ) ))
		.attr( axes.y, elt => min( scales.level( elt.level + e ), scales.level( elt.level - e ) ))
		.attr( axes.width, elt => abs( scales.position( elt.end ) - scales.position( elt.start )))
		.attr( axes.height, elt => abs( scales.level( -2 * e ) - scales.level(0) ))
		//.attr( 'stroke', 'black' )
		.attr( 'fill', 'grey' )
	;

	function isVisible( x ) {
		return x >= region.start && x <= region.end ;
	}
	// start bar
	transcripts
		.selectAll( 'line.bar_s' )
		.filter( elt => isVisible(elt.txStart))
		.attr( axes.x1, elt => scales.position( elt.txStart ))
		.attr( axes.x2, elt => scales.position( elt.txStart ))
		.attr( axes.y1, elt => scales.level( elt.level - h ))
		.attr( axes.y2, elt => scales.level( elt.level + H ) - 0.5 )
		.attr( 'stroke', 'black' )
	;
	// end bar
	transcripts
		.selectAll( 'line.bar_e' )
		.filter( elt => isVisible( elt.txEnd ))
		.attr( axes.x1, elt => scales.position( elt.txEnd ))
		.attr( axes.x2, elt => scales.position( elt.txEnd ))
		.attr( axes.y1, elt => scales.level( elt.level - h ))
		.attr( axes.y2, elt => scales.level( elt.level + h ))
		.attr( 'stroke', 'black' )
	;
	// start dash
	transcripts
		.selectAll( 'line.dash_s' )
		.filter( elt => !isVisible(elt.start))
		.attr( axes.x1, elt => scales.position( region.start ) - 10 )
		.attr( axes.x2, elt => scales.position( region.start ))
		.attr( axes.y1, elt => scales.level( elt.level ))
		.attr( axes.y2, elt => scales.level( elt.level ))
		.attr( 'stroke-dasharray', '2' )
		.attr( 'stroke', 'black' )
	;
	// end dash
	transcripts
		.selectAll( 'line.dash_e' )
		.filter( elt => !isVisible( elt.end ))
		.attr( axes.x1, elt => scales.position( region.end ) )
		.attr( axes.x2, elt => scales.position( region.end ) + orient * 10 )
		.attr( axes.y1, elt => scales.level( elt.level ))
		.attr( axes.y2, elt => scales.level( elt.level ))
		.attr( 'stroke-dasharray', '2' )
		.attr( 'stroke', 'black' )
	;

	// arrow
	transcripts
		.selectAll( 'line.arrow_body' )
		.attr( axes.x1, elt => scales.position( elt.txStart ))
		.attr( axes.x2, elt => scales.position( elt.txStart ) + orient * a[elt.strand] * 4 )
		.attr( axes.y1, elt => scales.level( elt.level + H ))
		.attr( axes.y2, elt => scales.level( elt.level + H ))
		.attr( 'stroke', 'black' )
	;
	transcripts
		.selectAll( 'line.arrow_lower' )
		.attr( axes.x1, elt => scales.position( elt.txStart ) + orient * a[elt.strand] * 4 )
		.attr( axes.x2, elt => scales.position( elt.txStart ) + orient * a[elt.strand] * 2 )
		.attr( axes.y1, elt => scales.level( elt.level + H ))
		.attr( axes.y2, elt => scales.level( elt.level + H ) - 2 )
		.attr( 'stroke', 'black' )
	;
	transcripts
		.selectAll( 'line.arrow_upper' )
		.attr( axes.x1, elt => scales.position( elt.txStart ) + orient * a[elt.strand] * 4 )
		.attr( axes.x2, elt => scales.position( elt.txStart ) + orient * a[elt.strand] * 2 )
		.attr( axes.y1, elt => scales.level( elt.level + H ))
		.attr( axes.y2, elt => scales.level( elt.level + H ) + 2 )
		.attr( 'stroke', 'black' )
	;
	
	transcripts
		.selectAll( 'text.symbol' )
		.attr( 'transform', function( elt ) {
			if( direction == "horizontal" ) {
				return "translate(" + (scales.position( elt.end ) + t) + ", " + scales.level( elt.level ) + ")" ;
			} else if( direction == "vertical" ) {
				return "translate(" + scales.level( elt.level ) + ", " + (scales.position( elt.start ) + t) + ") rotate(90)" ;
			}
		})
//		.attr( axes.x, elt => scales.position( elt.end ) + t )
//		.attr( axes.y, elt => scales.level( elt.level ))
		.attr( 'alignment-baseline', 'middle' )
		.attr( 'text-anchor', 'start' )
		.attr( 'font-size', elt => ( elt.symbol == elt.ID ? '7pt' : '7pt' ))
		.attr( 'font-weight', elt => ( elt.highlight == 1 ) ? 'bold' : 'normal' )
		.attr( 'fill', elt => ( elt.highlight == 1 ) ? aes.colour.highlight : aes.colour.text )
		.attr( 'font-style', 'italic' )
		.text( elt => elt.ID ) //( elt.name === null ? elt.ID : elt.symbol  ).replace( /P[Ff][A-Z0-9]*_/, '' ))
	;
}
