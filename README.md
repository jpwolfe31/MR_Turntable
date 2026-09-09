# MR_Turntable
This is a design and code to power a Walthers 90 foot HO model railroad rotary turntable.  

The turntable is powered by a 5V 1.8 degree unipolar stepper motor having a 1/2 inch rubber wheel riding against a 12 inch wooden wheel under the turntable.. This wooden wheel is attached to the pivot point of the bridge using a 1/4 inch OD hollow tube (the "bridge tube").  Position is controlled by a 2000 pulse per revolution XYZ rotary encoder attached to the bottom of the bridge tube.  The positions of the tracks are referenced to the encoder's Z index.  Using software, the encoder can be configured to identify 8,000 positions per revolution.

The bridge tube is attached to the center of the bridge with a hex key collar that has been epoxied inside the bridge.  There is a hole in the side of the bridge to tighten the bridge on the bridge tube.  The wooden wheel is attached to the bridge tube using hex key collars glued to each side of the wheel.  If I were doing this again, I would use two hex key collars with flanges and bolts instead of glued collars.    

Wires run through the tube just above the wooden wheel to power the track.  A DPDT relay controls the polarity of the bridge track as it rotates.  The turntable does not have a commutator for track power so the bridge cannot rotate beyond the encoder index point.

The current set up uses an Arduino UNO, a proto board for the connector to the stepper, encoder and relay and an LCD keypad shield.  There is also another board under the turntable with a motor driver circuit and a relay.  The motor drive circuit could be replaced with an Arduino motor shield with minimal software changes.  The track power relay could also be handled in different ways as well.

The main innovation here is the 12 inch wooden drive wheel which allows for very precise control of turntable position with no backlash.  The rubber wheel is just a piece of rubber tubing (in the current case, heavy electrical wire cover) over the 1/4 inch stepper motor shaft.  The turntable makes a rather nice low frequency rumbling noise as it rotates.  The bridge can be adjusted with the bridge hex key collar and by moving the position of the encoder relative to the turntable center.  The encoder cross support has enlarged holes at its ends to permit this fine adjustment.  I found that pushing the encoder up a bit while tightening the bridge down helps hold the bridge on the track in the turntable pit.

The turntable buttons include up/down for index positions up/down and left/right for fine tune steps left/right should that be needed.  The display shows current position and goto position.   It takes about a 30 seconds for a 180 degree rotation.

