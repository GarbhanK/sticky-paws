# Sticky Paws

### TODOs
**gameplay**
- [x] majorly clean up and finalise the SPEED logic, works now but old code left over that needs cleaning
- [x] Vectorise the dy/dx mouse movement speed, I think moving diagonals is way faster than across
- [x] add logic for the old man, different stages of awake
- [x] proper debug toggling logic
- [x] figure out what i'm going to do with the bar in the top right
- [x] Ensure sticky members are reset to false when restarted after fail
- [x] Stuck obj acting on unstuck obj, stuck object acting like it's being moved too. need to investigate/fix
- [x] Countdown timer until loss state

**visuals**
- [x] Add bear nose and associated vars to Bear struct
- [x] add title screen with real image of a bear (make sure start button places paw at the bottom)
- [x] add art for old man, maybe have him framed in a cool way? Old school TV with the antennas? courage the cowardly dog
- [x] some kind of narrative framing around the bear seeing the guy? Periscope?
- [x] rules explainer in the main menu
- [x] get all of the text in the right place
- [x] split drawUI() into separate parts

**sfx**
- [x] bear growls
- [x] sniffing nose
- [x] snoring/sleeping
- [x] obj movement
- [x] UI noises (button click)
- [x] Win/Fail music
- [x] Win/Fail sfx

**maybes**
- [ ] possibly replace jar of honey with a salmon
- [ ] add win animations of bear pics moving across screen overlapped
- [ ] gameplay options menu

**code**
- [x] add const to relevant vars
- [ ] change to `size_t` instead of `int` where necessary
- [x] order struct members smallest to biggest

**art**
- [ ] make more things sticky (jammy baguette, cinnamon bun)
- [ ] turn off the tv
- [ ] add in the gun (randomly goes off)
