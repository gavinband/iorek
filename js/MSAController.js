'use strict' ;

let MSAController = function( elt, view ) {
	this.elt = elt ;
	this.view = view ;
	this.drag = { start: 0 } ;
	let self = this ;
	let doDrag = function( x ) {
		let here = self.view.scales.x.invert( x ) ;
		let drag = here - self.drag.start ;
		let viewport = self.view.viewport() ;
		drag = Math.min( drag, viewport[0] - self.view.msa.scales.global.range()[0] + 50 ) ;
		drag = Math.max( drag, viewport[1] - self.view.msa.scales.global.range()[1] - 50 ) ;
		//console.log( "     DRAG", self.drag, self.viewport, self.view.msa.scales.global.range(), drag ) ;
		self.view.setViewport( [ viewport[0] - drag, viewport[1] - drag ] ) ;
	} ;
	
	// set up dragging
	elt.on( "mousedown", function( e ) {
//			self.dragging = true ;
//			self.dragStartRegion = self.viewport ;
//			self.dragStartCentre = (self.viewport[1] + self.viewport[0])/2 ;
		self.drag.start = self.view.scales.x.invert( e.offsetX ) ;
		self.dragging = true ;
		//console.log( "START DRAG", e.offsetX, e.offsetY, self.drag ) ;
	}) ;
	elt.on( "mousemove", function( e ) {
		if( self.dragging ) {
			doDrag( e.offsetX ) ;
		}
	}) ;
	elt.on( "mouseup", function(e) {
		if( self.dragging ) {
			doDrag( e.offsetX ) ;
			self.dragging = false ;
		}
	}) ;
	elt.on( "mouseout", function(e) {
		if( self.dragging ) {
			doDrag( e.offsetX ) ;
			self.dragging = false ;
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
	elt.on( 'mousewheel', function(e) {
		let viewport = self.view.viewport() ;
		let focus = self.view.scales.x.invert( e.offsetX ) ;
	//	console.log( "zooming:", e.wheelDeltaY, viewport, focus, self.view.msa ) ;
		
		if( e.wheelDeltaY > 0 && (viewport[1] - viewport[0]) > 100 ) {
			let scale = Math.pow( 2, Math.min( e.wheelDeltaY, 50.0 ) / 50.0 ) ;
			self.view.setViewport(
				zoomRegion( viewport, focus, scale )
			) ;
		}
		else if( e.wheelDeltaY < 0 && (viewport[1] - viewport[0]) < ( self.view.msa.alignmentLength )) {
			let scale = Math.pow( 2, Math.min( -e.wheelDeltaY, 50.0 ) / 50.0 ) ;
			let viewport = zoomRegion( self.view.viewport(), focus, 1/scale ) ;
			viewport[0] = Math.max( viewport[0], 0 ) ;
			viewport[1] = Math.min( viewport[1], self.view.msa.alignmentLength ) ;
			self.view.setViewport( viewport ) ;
		}
		e.stopPropagation() ;
		return false ;
	}) ;

	window.addEventListener( 'resize', function() {
		self.view.draw( true ) ;
	} ) ;

	return this ;
} ;
