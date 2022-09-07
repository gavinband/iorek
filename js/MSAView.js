'use strict' ;

let addRange = function( range1, range2 ) {
	return [ range1[0] + range2[0], range1[1] + range2[1] ] ;
}

let getPositiveRange = function( range ) {
	return [
		Math.min( range[0], range[1] ),
		Math.max( range[0], range[1] )
	] ;
}

let MSAView = function(
	elt,
	msa,
	reference,
	genes,
	annotations,
	geom = {
		layout: {
			heights: {
				base: 8,
				sequence: 10,
				annotation: 12,
				genes: 100,
				min_per_sequence: 15
			},
			width: {
				all: window.innerWidth - 40,
				names: 200,
				sequences: window.innerWidth - 240,
				reference: window.innerWidth - 240
			}
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
	this.reference.orientation = Math.sign( reference.coordinateRange.end - reference.coordinateRange.start ) ;
	this.reference.baseMargin = (this.reference.orientation == 1) ? [-0.5, 0.5] : [1.5, 0.5] ;
	//console.log( "R", this.reference ) ;
	this.genes = genes ;
	this.annotations = annotations ;
	this.target = "sequence" ;
	
	geom.layout.heights.genes = Math.max(
		geom.layout.heights.genes,
		20 * genes.numberOfLevels
	) ;

	this.geom = geom ;

	this.scales = function(){
		let result = {} ;
		result.tracks = new AlignmentTrackScale(
			msa,
			annotations,
			geom
		) ;
		result.y = new d3.scaleLinear()
			.domain( [ 0, result.tracks.total_height() ])
			.range( [ 0, result.tracks.total_height() ] ) ;
		// adjust ranges by half a basewidth, so that bases line up on coordinates.
		result.msaToX = new d3.scaleLinear()
			.domain( addRange( msa.scales.global.range(), [-0.5, 0.5] ))
			.range( [ geom.margin.left, geom.layout.width.sequences - geom.margin.right ] ) ;
		//console.log( "RCR", reference ) ;
		let orientation = reference.orientation ;
		result.concatenatedToX = new d3.scaleLinear()
			.domain( [ 0, reference.sequence.length ] )
			.range( result.msaToX.range() ) ;
		result.physicalToConcatenated = new d3.scaleLinear()
			.domain( [ reference.coordinateRange.start, reference.coordinateRange.end + 1 ] )
			.range( result.concatenatedToX.domain() ) ;
		result.physicalToX = d3.scaleLinear()
			.domain( result.physicalToConcatenated.domain() )
			.range( result.concatenatedToX.range() ) ;
		result.genes = new d3.scaleLinear()
			.domain( [Math.max( genes.numberOfLevels, 1 ), 0] )
			.range( [geom.margin.top, geom.layout.heights.genes - geom.margin.bottom - 10 ] ) ;
		return result ;
	}() ;

	this.aes = {
		bases: {
			colour: {
				'-': 'lightgrey',
				'm': '#333618',
				'a': '#02AAE9',
				't': '#1A356C',
				'c': '#941504',
				'g': '#F44B1A',
				'n': '#555555',
				'A': '#02AAE9',
				'T': '#1A356C',
				'C': '#941504',
				'G': '#F44B1A',
				'N': '#555555'
			},
			geom: {
				'-': { "offset": -5, "height": 2 },
				'm': { "offset": -8, "height": 8 },
				'a': { "offset": -8, "height": 8 },
				't': { "offset": -8, "height": 8 },
				'c': { "offset": -8, "height": 8 },
				'g': { "offset": -8, "height": 8 },
				'n': { "offset": -6, "height": 6 },
				'A': { "offset": -8, "height": 8 },
				'T': { "offset": -8, "height": 8 },
				'C': { "offset": -8, "height": 8 },
				'G': { "offset": -8, "height": 8 },
				'N': { "offset": -6, "height": 6 },
			}
		},
		colour: {
			"text": "#eeeeee",
			"highlight": "#FF4444",
			"background": '#222222'
		}
	} ;
	
	this.drawn_viewport = [ 0, 0 ] ;
	
	console.log( "++ MSAView(): scales.reference = ", this.scales.concatenatedToX.domain(), this.scales.concatenatedToX.range() ) ;
	
	return this ;
}


MSAView.prototype.viewport = function() {
	return this.scales.msaToX.domain() ;
}

MSAView.prototype.setViewport = function( viewport ) {
	//assert( end >= start && start >= 0 ) ;
	this.scales.msaToX.domain( viewport ) ;
	
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
	let orientation = this.reference.orientation ;

	let refViewport = [
		lr.invert( Math.max( viewport[0], lr.range()[0] )), // clip to left edge
		rr.invert( Math.min( viewport[1], rr.range()[1] ))  // clip to right edge
	] ;

	let physicalRange = addRange( refViewport, this.reference.baseMargin ) ;
	this.set_physical_domain( physicalRange ) ;
} ;

MSAView.prototype.set_physical_domain = function( domain ) {
	let concatenated_domain = [
		Math.max( Math.floor( this.scales.physicalToConcatenated( domain[0] )), 0 ),
		Math.min( Math.ceil( this.scales.physicalToConcatenated( domain[1] )), this.reference.sequence.length )
	] ;
	this.scales.concatenatedToX.domain( concatenated_domain ) ;
	this.scales.physicalToX
		.domain( [
			this.scales.physicalToConcatenated.invert( concatenated_domain[0] ),
			this.scales.physicalToConcatenated.invert( concatenated_domain[1] )
		] ) ;
}

MSAView.prototype.updateLayout = function() {
	let geom = this.geom ;
	geom.layout.width.all = window.innerWidth - 40 ;
	geom.layout.width.sequences = window.innerWidth - geom.layout.width.names - 40 ;
	geom.layout.width.reference = window.innerWidth - geom.layout.width.names - 40 ;
	this.scales.msaToX.range( [ geom.margin.left, geom.layout.width.sequences - geom.margin.right ] ) ;
	this.scales.concatenatedToX.range( this.scales.msaToX.range() ) ;
}

MSAView.prototype.draw = function( force ) {
	let viewport = this.scales.msaToX.domain() ;
	if( !force && (this.drawn_viewport[0] == viewport[0] && this.drawn_viewport[1] == viewport[1] )) {
		return ;
	}
	let panels = this.panels ;
	// Ensure we fit the screen.
	this.updateLayout() ;
	let geom = this.geom ;
	let vs = this.scales ;
	let aes = this.aes ;

	let canvas_height = vs.tracks.total_height() + geom.margin.top + geom.margin.bottom  ;

	vs.y.range( [ canvas_height - geom.margin.bottom, geom.margin.top ] ) ;
	// put panels in the right place
	d3.select( '.figure' )
		.attr( 'width', geom.layout.width.all )
		.attr( 'height', canvas_height + 300 ) ;

	panels.names.attr( 'width', geom.layout.width.names ) ;
	panels.names.attr( 'height', canvas_height ) ;

	panels.sequences.attr( 'width', geom.layout.width.sequences ) ;
	panels.sequences.attr( 'height', canvas_height ) ;

	panels.genes.style( 'top', canvas_height + 40 ) ;
	panels.genes.attr( 'width', geom.layout.width.reference ) ;

	panels.controls.style( 'top', '20px' ) ;
	panels.controls.attr( 'height', '30px' ) ;
	panels.controls.attr( 'width', geom.layout.width.reference ) ;

	panels.sequences.selectAll('*').remove() ;
	
	{
		let formatName = function( name ) {
			if( name == "reference" ) {
				return "(concatenated)" ;
			} else {
				return name ;
			}
		}
		let renderName = function( selection ) {
			selection
				.append( 'text' )
				.attr( 'x', 0 )
				.attr( 'y', 0 )
				.attr( 'text-anchor', 'end' )
				.attr( 'dominant-baseline', 'middle' )
				.attr( 'font-size', d => (d.track_name == 'sequence') ? '12px' : '8px' )
				.attr( 'dy', d => (d.track_name == 'sequence') ? (-aes.bases.geom['a'].height/2) : (-geom.layout.heights.annotation/2) )
				.attr( 'font-family', 'Courier' )
				.attr( 'font-style', 'italic' )
				.attr( 'fill', aes.colour.text )
				.text( d => (d.track_name == 'sequence') ? formatName( d.sequence_id ) : d.track_name ) ;
		} ;
		let names = panels.names.selectAll( 'g.name' )
			.data( vs.tracks.tracks() )
			.enter()
			.append( 'g' )
			.attr( "class", "name" )
			.attr( 'transform', ( d, i ) => ('translate(' + (geom.layout.width.names - geom.margin.right) + ',' + vs.y(vs.tracks.map(d.sequence_id, d.track_name).baseline) + ")" ))
		;
		names.call( renderName ) ;
	}

	{
		let guide = panels.controls.selectAll( 'g.guide' )
			.data( ['guide'] )
			.enter()
			.append( 'g' )
			.attr( 'class', 'guide' )
			.attr( 'transform', (d,i) => ('translate(' + (geom.layout.width.reference/2) + ", 10)" ))
			.append( 'text' ) ;
		guide
			.append( 'tspan' )
			.attr( 'font-size', '10pt' )
			.attr( 'font-family', 'Palatino' )
			.attr( 'dominant-baseline', 'middle' )
			.attr( 'text-anchor', 'middle' )
			.text( "Use mouse to scroll and zoom" )
		;
	}
	{
		let logo = panels.controls.selectAll( 'g.logo' )
			.data( ['seemsa'] )
			.enter()
			.append( 'g' )
			.attr( 'class', 'logo' )
			.attr( 'transform', (d,i) => ('translate(' + (geom.layout.width.reference - 100) + ", 10)" ))
			.append( 'text' ) ;
		logo
			.append( 'tspan' )
			.attr( 'font-size', '10pt' )
			.attr( 'font-family', 'Helvetica' )
			.attr( 'dominant-baseline', 'middle' )
			.text( d => d ) ;
		logo
			.append( 'tspan' )
			.attr( 'font-size', '6pt' )
			.attr( 'font-family', 'Palatino' )
			.attr( 'font-style', 'italic' )
			.attr( 'dominant-baseline', 'middle' )
			.attr( 'baseline-shift', 'super' )
			.text( "beta" ) ;

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
			.attr( 'fill', d => this.aes.bases.colour[d] ) ;
		panels.controls.selectAll( 'g.base' )
			.append( 'text' )
			.attr( 'x', 10 )
			.attr( 'y', 0 )
			.attr( 'font-size', '10pt' )
			.attr( 'font-family', 'Palatino' )
			.attr( 'dominant-baseline', 'middle' )
			.text( d => d.toUpperCase() ) ;
			
		let switches = panels.controls.selectAll( 'g.control' )
			.data( ['highlight mismatches?'] )
			.enter()
			.append( 'g' )
			.attr( 'class', 'control' )
			.attr( 'transform', "translate(180,5)") ;
		switches
			.append( 'rect' )
			.attr( 'class', 'checkbox' )
			.attr( 'checked', 'false' )
			.attr( 'x', 0 )
			.attr( 'y', 0 )
			.attr( 'width', 8 )
			.attr( 'height', 8 )
			.attr( 'stroke', aes.colour.text )
			.attr( 'fill', '#111111' ) ;
		switches.append( 'path' )
			.attr( 'd', 'M1 1 L7 7 M7 1 L1 7')
			.attr( 'stroke', '#111111' ) ;
		switches
			.append( 'text' )
			.attr( 'x', 12 )
			.attr( 'y', 4 )
			.attr( 'alignment-baseline', 'central' )
			.attr( 'font-size', '8pt' )
			.text( d => d ) ;
	}
	{
		let genes = this.genes ;
		//console.log( "GENES", this.genes ) ;
		genes.draw(
			panels.genes, {
				position: vs.concatenatedToX,
				level: vs.genes
			}
		) ;
	}
	{
		let axis = panels.genes.selectAll( 'g.axis' ).data( ['axis'] ) ;
		axis
			.enter()
			.append( 'g' )
			.attr( 'class', 'axis' )
		let axisBottom = d3.axisBottom( vs.physicalToX ) ;
		panels.genes.selectAll( 'g.axis' )
			.attr( 'transform', 'translate( 0,' + ( vs.genes.range()[1] + 10 ) + ')' )
			.call( axisBottom ) ;
	}

	let drawSequence = function( sequence, levels, y, baseWidth, xScale, range, ctx ) {
		// calculate stride across sequence as follows.
		// Fir, we aim to have every drawn rectangle of some minimum size below which
		// we aggregate.  
		let visibleMarkWidth = 3.0 ;
		// Our aggregation happens at power-of-2 sizes (c.f. MSA.js / computeLevels)
		// Therefore we compute the power-of-two stride that is less than or equal to the
		// number of bases that fit into the visible width.
		let by = Math.pow(
			2,
			Math.floor(
				Math.max(
					0,
					Math.log2( visibleMarkWidth / baseWidth )
				)
			)
		) ;
		by = Math.min( by, 1024 ) ;
		//by = Math.max( 1, Math.floor( 1.0/baseWidth )) ;
		// leave a small gap between bases, but
		// never draw less than one pixel wide
		let visualBaseWidth = Math.max( baseWidth * ( by - 0.01 ), 1 ) ;
		let j = range[0] ;
		let level_j = Math.floor( j/by ) ;
		let base = 'A' ;
		console.log( "by", by, level_j ) ;
		for( ; j < range[1]; j += by, level_j += 1 ) {
			if( by > 1 && levels !== null ) {
				base = levels[by][level_j] ;
			} else {
				base = sequence[j] ;
			}
			let geom ;
			if( aes.bases.geom.hasOwnProperty( base )) {
				geom = aes.bases.geom[base] ;
			} else {
				geom = aes.bases.geom['-'] ;
			}
			ctx.fillStyle = aes.bases.colour[base] || aes.bases.colour['-'];
			ctx.fillRect(
				xScale( j ),
				y + geom.offset,
				visualBaseWidth,
				geom.height
			) ;
		}
	} ;

	{
		let sequences = panels.sequences.node() ;
		let ctx = sequences.getContext( '2d' ) ;
		let baseWidth = (vs.msaToX(1) - vs.msaToX(0)) ;
		console.log( baseWidth, this.target ) ;
		let msa = this.msa ;
		// only draw bases in the viewport
		let visible_range = [
			Math.max( Math.floor( viewport[0] ), 0 ),
			Math.min( Math.ceil( viewport[1] + 1 ), msa.alignment[0].sequence.length )
		] ;
		for( let i = 0; i < msa.alignment.length; ++i ) {
			let sequence = msa.alignment[i].sequence ;
			drawSequence(
				msa.alignment[i][ this.target ],
				msa.alignment[i][ this.target ].levels,
				vs.y(vs.tracks.map(msa.alignment[i].name, "sequence").baseline),
				baseWidth,
				j => vs.msaToX( msa.scales.global(j)),
				visible_range,
				ctx
			) ;
		}
		
		let orientation = this.reference.orientation ;
		let baseOffset = ((orientation==-1)?-1:0) ;
		//console.log( "DRAWING REFERENCE, starting at ", vs.concatenatedToX(this.reference.coordinateRange.start )) ;
		let referenceBaseWidth = Math.abs(
			vs.concatenatedToX(this.reference.coordinateRange.start+orientation)
			- vs.concatenatedToX(this.reference.coordinateRange.start)
		) ;
		drawSequence(
			this.reference.sequence,
			null,
			vs.y(vs.tracks.map( "reference", "sequence" ).baseline),
			referenceBaseWidth,
			this.scales.concatenatedToX,
			//j => ( vs.concatenatedToX(this.reference.coordinateRange.start + orientation*j - baseOffset )),
			//0,
			//this.reference.sequence.length,
			this.scales.concatenatedToX.domain(),
			ctx
		) ;
		
		{
			let gs = msa.scales.global ;
			let yoffset = 2 ;
			let ys = {
				"a": vs.y(vs.tracks.map( this.reference.name, "sequence" ).baseline) + yoffset,
				"b": vs.y(vs.tracks.map( "reference", "sequence" ).baseline) - geom.layout.heights.base - yoffset
 			} ;
			// plot indels between joined and msa reference
			// and plot axis tick points
			
			let ticks = vs.physicalToX.ticks() ;
			
			let last_x00 = -100000000 ;
			for( let i = 0; i < this.reference.ranges.length; ++i ) {
				let range = this.reference.ranges[i] ;
				//console.log( "R", range ) ;
				let x00 = vs.msaToX( gs( range.inAlignment.start )) ;
				if( (x00 - last_x00) > 5 ) {
					let x01 = vs.physicalToX( range.inSequence.start - baseOffset ) ;
					let x11 = vs.physicalToX( range.inSequence.end  - baseOffset ) ;
					let x10 = vs.msaToX( gs( range.inAlignment.end )) ;

					ctx.beginPath() ;
					// ref sequence is 2 * heights.sequence down.
					ctx.moveTo( x00, ys.a ) ;
					ctx.lineTo( x01, ys.b ) ;
					ctx.moveTo( x11, ys.b ) ;
					ctx.lineTo( x10, ys.a ) ;
					ctx.strokeStyle = "grey" ;
					ctx.stroke() ;
					last_x00 = x00 ;
					
					let rangeTicks = ticks.filter( t => (t >= range.inSequence.start && t <= range.inSequence.end) ) ;
					for( let j = 0; j < rangeTicks.length; ++j ) {
						let x0 = vs.msaToX( range.inAlignment.start + (rangeTicks[j] - range.inSequence.start) + 0.5 ) ;
						let x1 = vs.physicalToX( rangeTicks[j] ) ;
						ctx.beginPath() ;
						// ref sequence is 2 * heights.sequence down.
						ctx.moveTo( x0, ys.a ) ;
						ctx.lineTo( x1, ys.b ) ;
						ctx.strokeStyle = "grey" ;
						ctx.stroke() ;
					}
				}
			}
		}
	}
	
	{
		let orientation = this.reference.orientation ;
		let sequences = panels.sequences.node() ;
		let ctx = sequences.getContext( '2d' ) ;
		for( let sequence_id in this.annotations ) {
			let sequence_annotations = this.annotations[sequence_id] ;
			let ranges = this.msa.scales.ranges[sequence_id] ;
			for( let track_name in sequence_annotations ) {
				let track = sequence_annotations[track_name] ;
				ctx.beginPath() ;
				ctx.moveTo(
					geom.margin.left,
					vs.y( vs.tracks.map( sequence_id, track_name ).baseline )
				) ;
				ctx.lineTo(
					geom.layout.width.all - geom.margin.right,
					vs.y( vs.tracks.map( sequence_id, track_name ).baseline )
				) ;
				ctx.lineWidth = 0.5 ;
				ctx.strokeStyle = '#558855' ;
				ctx.stroke() ;
				for( let i = 0; i < ranges.length; ++i ) {
					let sequenceToAlignment = ranges[i] ;
					let domain = getPositiveRange( sequenceToAlignment.domain() );
					let range_track = track.filter( elt => elt.end >= domain[0] && elt.start <= domain[1] ) ;
					for( let j = 0; j < range_track.length; ++j ) {
						let a = range_track[j] ;
						let rd = {
							x1: vs.msaToX( sequenceToAlignment( Math.max( a.start, domain[0] ))),
							y1: vs.y( vs.tracks.map( sequence_id, track_name ).baseline + vs.tracks.scale_value( sequence_id, track_name, a.value ) ),
							x2: vs.msaToX( sequenceToAlignment( Math.min( a.end, domain[1] )) + orientation ),
							y2: vs.y( vs.tracks.map( sequence_id, track_name ).baseline)
						} ;
						rd.xl = Math.min( rd.x1, rd.x2 ) ;
						rd.xr = Math.max( rd.x1, rd.x2 ) ;
						//console.log( "RD", rd ) ;
						ctx.fillStyle = '#6a6a6a' ;
						ctx.fillRect(
							rd.xl,
							rd.y1,
							rd.xr - rd.xl,
							rd.y2 - rd.y1
						) ;
					}
				} 
			}
		}
	}
	
	this.drawn_viewport[0] = viewport[0] ;
	this.drawn_viewport[1] = viewport[1] ;
}
