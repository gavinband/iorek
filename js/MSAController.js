'use strict' ;

let MSAController = function(
	panels,
	view,
	url_params
) {
	this.panels = panels ;
	this.view = view ;
	this.drag = { start: 0 } ;
	this.params = url_params ;
	let self = this ;
	let doDrag = function( x ) {
		let here = self.view.scales.msaToX.invert( x ) ;
		let drag = here - self.drag.start ;
		let viewport = self.view.viewport() ;
		drag = Math.min( drag, viewport[0] - self.view.msa.scales.global.range()[0] + 50 ) ;
		drag = Math.max( drag, viewport[1] - self.view.msa.scales.global.range()[1] - 50 ) ;
		//console.log( "     DRAG", self.drag, self.viewport, self.view.msa.scales.global.range(), drag ) ;
		self.view.setViewport( [ viewport[0] - drag, viewport[1] - drag ] ) ;
	} ;
	
	panels.controls.on( "click", function( e ) {
		let checkbox = document.getElementById( 'mismatch_mode_toggle' ) ;
		let share = document.getElementById( 'share_button' ) ;
		if( checkbox.contains( e.target )) {
			console.log( checkbox ) ;
			if( checkbox.getAttribute( 'checked' ) == 'false' ) {
				checkbox.setAttribute( 'checked', 'true' ) ;
				d3.select(checkbox).selectAll( 'rect' ).attr( 'fill', '#dddddd' ) ;
				self.view.target = "mismatches" ;
			} else {
				d3.select(checkbox).selectAll( 'rect' ).attr( 'fill', '#111111' ) ;
				checkbox.setAttribute( 'checked', 'false' ) ;
				self.view.target = "sequence" ;
			}
		}
		else if( share.contains( e.target )) {
			self.updateParams() ;
			let location = window.location ;
			let url = location.protocol + "://" + location.host + ( location.port ? (":" + location.port) : "" ) + location.pathname + "?" + self.params.toString() ;
			navigator.clipboard.writeText( url ) ;
			history.pushState( null, null, "?" + self.params.toString() ) ;
			console.log( "Copied to clipboard", url ) ;
		}
		self.updateParams() ;
		self.view.draw( true ) ;
	}) ;
	
	// set up dragging
	panels.sequences.on( "mousedown", function( e ) {
//			self.dragging = true ;
//			self.dragStartRegion = self.viewport ;
//			self.dragStartCentre = (self.viewport[1] + self.viewport[0])/2 ;
		self.drag.start = self.view.scales.msaToX.invert( e.offsetX ) ;
		self.dragging = true ;
		//console.log( "START DRAG", e.offsetX, e.offsetY, self.drag ) ;
	}) ;
	panels.sequences.on( "mousemove", function( e ) {
		if( self.dragging ) {
			doDrag( e.offsetX ) ;
		}
	}) ;
	panels.sequences.on( "mouseup", function(e) {
		if( self.dragging ) {
			doDrag( e.offsetX ) ;
			self.dragging = false ;
			self.updateParams() ;
		}
	}) ;
	panels.sequences.on( "mouseout", function(e) {
		if( self.dragging ) {
			doDrag( e.offsetX ) ;
			self.dragging = false ;
			self.updateParams() ;
		}
	}) ;
	
	let zoomRegion = function( viewport, focus, zoom ) {
		// Expand or contract region around given centre point
		// which must lie in the region.
		// The way this works is that the chosen point stays fixed on the screen
		// And everything else zooms.
		// zoom=1 means no zoom
		// zoom>1 means zoom in
		// zoom<1 means zoom out
		return(
				[
					focus + ((viewport[0] - focus) / zoom),
					focus + ((viewport[1] - focus) / zoom)
				]
		) ;
	} ;
	// Set up zooming
	panels.sequences.on( 'mousewheel', function(e) {
		let viewport = self.view.viewport() ;
		let focus = self.view.scales.msaToX.invert( e.offsetX ) ;
	//	console.log( "zooming:", e.wheelDeltaY, viewport, focus, self.view.msa ) ;
		
		let scale = Math.pow( 2, Math.min( Math.abs(e.wheelDeltaY), 50.0 ) / 50.0 ) ;
		if( ( e.wheelDeltaY > 0 ) && (viewport[1] - viewport[0]) > 25 ) {
			viewport = zoomRegion( viewport, focus, scale ) 
		}
		else if( e.wheelDeltaY < 0 && (viewport[1] - viewport[0]) < ( self.view.msa.alignmentLength )) {
			viewport = zoomRegion( viewport, focus, 1/scale ) ;
		}
		viewport[0] = Math.max( viewport[0], -0.5 ) ;
		viewport[1] = Math.min( viewport[1], self.view.msa.alignmentLength - 0.5 ) ;
		self.view.setViewport( viewport ) ;
		self.updateParams() ;
		e.stopPropagation() ;
		return false ;
	}) ;

	window.addEventListener( 'resize', function() {
		self.view.draw( true ) ;
	} ) ;

	let updateState = function(event) {
		self.params = new URLSearchParams( window.location.search ) ;
		console.log( "PARAMS", self.params ) ;
		self.interpretParams( self.params ) ;
		self.view.draw( true ) ;
	} ;
	window.addEventListener( 'popstate', updateState ) ;
	window.addEventListener( 'pushstate', updateState ) ;

	console.log( "PARAMS", self.params ) ;

	self.interpretParams( this.params ) ;
	self.view.draw( true ) ;

	return this ;
} ;

MSAController.prototype.interpretParams = function( params ) {
	if( params.has( "viewport" )) {
		let viewport = this.parseViewport( params.get( "viewport" ) ) ;
//		console.log( "VIEWPORT", viewport ) ;
		this.view.setViewport( viewport ) ;
	} else {
		this.view.setViewport( [ -0.5, this.view.msa.scales.alignmentLength - 0.5 ] ) ;
	}

	if( params.has( "target" )) {
		let target = params.get( "target" ) ;
		if( target == "sequence" || target == "mismatches" ) {
			this.view.target = target ;
		}
	} else {
		this.view.target = 'sequence' ;
	}
}

MSAController.prototype.updateParams = function() {
	this.params.set( "target", this.view.target ) ;
	this.params.set( "viewport", this.encodeViewport( this.view.viewport() )) ;
}

MSAController.prototype.parseViewport = function( spec ) {
//	console.log( "VIEWPORT SPEC", spec ) ;
	let bits = spec.split( ":" ) ;
	if( bits.length != 2 ) {
		return null ;
	}
	return [
		parseFloat( bits[0] ),
		parseFloat( bits[1] )
	] ;
}

MSAController.prototype.encodeViewport = function( viewport ) {
//	console.log( "VIEWPORT", viewport ) ;
	return Math.floor(viewport[0]) + ":" + Math.ceil( viewport[1] ) ;
}