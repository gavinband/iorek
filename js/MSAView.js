'use strict' ;

let MSAView = function(
	elt,
	msa,
	reference,
	genes,
	geom = {
		layout: {
			baseHeight: 8,
			sequenceHeight: 20,
			width: {
				all: window.innerWidth - 40,
				names: 200,
				sequences: window.innerWidth - 240,
				reference: window.innerWidth - 240
			},
			genePanelHeight: 100
		},
		margin: {
			top: 20,
			bottom: 20,
			left: 10,
			right: 10
		}
	}
) {
	this.elt = elt ;
	this.panels = {
		controls: elt.select( "svg.controls" ),
		names: elt.select( "svg.names" ),
		sequences: elt.select( "canvas.sequences" ),
		genes: elt.select( "svg.genes" )
	} ;
	this.msa = msa ;
	this.reference = reference ;

	this.genes = genes ;
	this.geom = geom ;
	this.geom.layout.height = ((msa.alignment.length + 2) * geom.layout.sequenceHeight) + geom.margin.top + geom.margin.bottom ;

	this.scales = function(){
		let result = {} ;
		result.y = new d3.scaleBand()
			.domain( [ "reference", "" ].concat( msa.alignment.map( d => d.name ) ))
			.range( [ geom.layout.height, geom.margin.top ] ) ;
		result.x = new d3.scaleLinear()
			.domain( msa.scales.global.range() )
			.range( [ geom.margin.left, geom.layout.width.sequences - geom.margin.right ] ) ;
		result.reference = new d3.scaleLinear()
			.domain( [ reference.coordinateRange.start, reference.coordinateRange.end ] )
			.range( result.x.range() ) ;
		result.alignmentReferenceMap = new d3.scaleLinear()
			.domain( msa.scales.global.range() )
			.range( result.reference.domain() )
		;
		result.genes = new d3.scaleLinear()
			.domain( [Math.max( genes.numberOfLevels, 1 ), 0] )
			.range( [geom.margin.top, geom.layout.genePanelHeight - geom.margin.bottom ] ) ;
		return result ;
	}() ;

	this.aes = {
		bases: {
			'-': { 'colour': 'lightgrey', "offset": -1, "height": 2 },
			'a': { 'colour': '#02AAE9', "offset": -4, "height": 8 },
			't': { 'colour': '#1A356C', "offset": -4, "height": 8 },
			'c': { 'colour': '#F44B1A', "offset": -4, "height": 8 },
			'g': { 'colour': '#941504', "offset": -4, "height": 8 }
		}
	} ;
	
	this.drawn_viewport = [ 0, 0 ] ;
	
	return this ;
}

MSAView.prototype.setViewport = function( viewport ) {
	//assert( end >= start && start >= 0 ) ;
	this.scales.x.domain( viewport ) ;
	let ranges = this.msa.scales.ranges[ this.reference.name ] ;
	let choice = { left: -1, right: -1 } ;
	for( let i = 0; i < ranges.length; ++i ) {
		let range = ranges[i] ;
		if( range.range()[1] >= viewport[0] && choice.left == -1 ) {
			choice.left = i ;
		}
		if( range.range()[0] < viewport[1] ) {
			choice.right = i ;
		}
	}
	let lr = ranges[choice.left] ;
	let rr = ranges[choice.right] ;
	
	let refViewport = [
		lr.invert( Math.max( viewport[0], lr.range()[0] )),
		rr.invert( Math.min( viewport[1], rr.range()[1] ))
	] ;

	this.scales.reference.domain( refViewport ) ;
} ;

MSAView.prototype.viewport = function() {
	// current view coordinates, in msa coordinates
	return this.scales.x.domain() ;
} ;

MSAView.prototype.updateLayout = function() {
	let geom = this.geom ;
	geom.layout.width.all = window.innerWidth - 40 ;
	geom.layout.width.sequences = window.innerWidth - geom.layout.width.names - 40 ;
	geom.layout.width.reference = window.innerWidth - geom.layout.width.names - 40 ;
	this.scales.x.range( [ geom.margin.left, geom.layout.width.sequences - geom.margin.right ] ) ;
}

MSAView.prototype.draw = function() {
	let viewport = this.viewport() ;
	if( this.drawn_viewport[0] == viewport[0] && this.drawn_viewport[1] == viewport[1] ) {
		return ;
	}
	let panels = this.panels ;
	// Ensure we fit the screen.
	this.updateLayout() ;
	let geom = this.geom ;
	let vs = this.scales ;
	let aes = this.aes ;

	// put panels in the right place
	d3.select( '.figure' )
		.attr( 'width', geom.layout.width.all )
		.attr( 'height', geom.layout.height + 300 ) ;

	panels.names.attr( 'width', geom.layout.width.names ) ;
	panels.names.attr( 'height', geom.layout.height ) ;

	panels.sequences.attr( 'width', geom.layout.width.sequences ) ;
	panels.sequences.attr( 'height', geom.layout.height ) ;

	panels.genes.style( 'top', geom.layout.height + 40 ) ;
	panels.genes.attr( 'width', geom.layout.width.reference ) ;

	panels.controls.style( 'top', '20px' ) ;
	panels.controls.attr( 'height', '20px' ) ;
	panels.controls.attr( 'width', geom.layout.width.reference ) ;

	panels.sequences.selectAll('*').remove() ;
	
	{
		let renderName = function( selection ) {
			selection
				.append( 'text' )
				.attr( 'x', 0 )
				.attr( 'y', 0 )
				.attr( 'text-anchor', 'end' )
				.attr( 'alignment-baseline', 'middle' )
				.attr( 'font-size', '10pt')
				.attr( 'font-family', 'Palatino' )
				.text( name => name ) ;
		} ;
		let names = panels.names.selectAll( 'g.name' )
			.data( data.alignment.map( elt => elt.name ).concat( [ "reference" ]) )
			.enter()
			.append( 'g' )
			.attr( "class", "name" )
			.attr( 'transform', ( name, i ) => ('translate(' + (geom.layout.width.names - geom.margin.right) + ',' + vs.y(name) + ")" ))
		;
		names.call( renderName ) ;
	}

	{
		let base_aes = this.aes.bases ;
		panels.controls.selectAll( 'g.base' )
			.data( [ 'a', 'c', 'g', 't' ] )
			.enter()
			.append( 'g' )
			.attr( 'class', 'base' )
			.attr( 'transform', (d,i) => ('translate(' + (20 + i*40) + ", 10)" )) ;
		panels.controls.selectAll( 'g.base' )
			.append( 'rect' )
			.attr( 'x', 0 )
			.attr( 'y', -5 )
			.attr( 'width', 6 )
			.attr( 'height', 10 )
			.attr( 'fill', d => base_aes[d].colour ) ;
		panels.controls.selectAll( 'g.base' )
			.append( 'text' )
			.attr( 'x', 10 )
			.attr( 'y', 0 )
			.attr( 'font-size', '10pt' )
			.attr( 'font-family', 'Palatino' )
			.attr( 'dominant-baseline', 'middle' )
			.text( d => d.toUpperCase() ) ;
	}
	{
		let genes = this.genes ;
		
		genes.draw(
			panels.genes, {
				position: vs.reference,
				level: vs.genes
			}
		) ;
		let axis = panels.genes.selectAll( 'g.axis' ).data( ['axis'] ) ;
		axis
			.enter()
			.append( 'g' )
			.attr( 'class', 'axis' )
		panels.genes.selectAll( 'g.axis' )
			.attr( 'transform', 'translate( 0,' + ( vs.genes.range()[1] - geom.margin.bottom + 2 ) + ')' )
			.call( d3.axisBottom( vs.reference )) ;
	}

	let drawSequence = function( sequence, y, baseWidth, xScale, lower, upper, ctx ) {
		let by = Math.max( 1, Math.floor( 1.0/baseWidth )) ;
		for( let j = lower; j < upper; j += by ) {
			let base = sequence[j] ;
			let base_aes = aes.bases[base] ;
			ctx.fillStyle = base_aes.colour ;
			ctx.fillRect(
				xScale( j ),
				y + base_aes.offset,
				Math.max( baseWidth, 1 ),
				base_aes.height
			) ;
		}
	} ;

	{
		let sequences = panels.sequences.node() ;
		let ctx = sequences.getContext( '2d' ) ;
		let baseWidth = vs.x(1) - vs.x(0) ;
		let msa = this.msa ;
		for( let i = 0; i < msa.alignment.length; ++i ) {
			let sequence = msa.alignment[i].sequence ;
			drawSequence(
				msa.alignment[i].sequence,
				vs.y(msa.alignment[i].name ),
				baseWidth,
				j => vs.x( msa.scales.global(j)),
				Math.max( Math.floor( viewport[0] ), 0 ),
				Math.min( Math.ceil( viewport[1] ), sequence.length ),
				ctx
			) ;
		}
		
		drawSequence(
			this.reference.sequence,
			vs.y( "reference" ),
			vs.reference(1) - vs.reference(0),
			j => vs.reference(this.reference.coordinateRange.start + j),
			0,
			this.reference.sequence.length,
			ctx
		) ;
		
		{
			let gs = msa.scales.global ;
			let yoffset = geom.layout.baseHeight + 2 ;
			let ys = {
				"a": vs.y( "reference" ) - (2 * (geom.layout.sequenceHeight - yoffset)),
				"b": vs.y( "reference" ) - yoffset
			} ;
			for( let i = 0; i < this.reference.ranges.length; ++i ) {
				let range = this.reference.ranges[i] ;
				let x00 = vs.x( gs( range.inAlignment.start )) ;
				let x01 = vs.reference( range.inSequence.start ) ;
				let x11 = vs.reference( range.inSequence.end ) ;
				let x10 = vs.x( gs( range.inAlignment.end )) ;
				ctx.beginPath() ;
				// ref sequence is 2 * sequenceHeight down.
				ctx.moveTo( x00, ys.a ) ;
				ctx.lineTo( x01, ys.b ) ;
				ctx.moveTo( x11, ys.b ) ;
				ctx.lineTo( x10, ys.a ) ;
				ctx.strokeStyle = "grey" ;
				ctx.stroke() ;
			}
		}
		
		this.drawn_viewport[0] = viewport[0] ;
		this.drawn_viewport[1] = viewport[1] ;
	}
}
