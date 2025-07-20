# Memory leak document

I was having an issue for ages where every time I exited the game I would get a crash dialog box. For the longest time I thought it was normal until I was running the more complex example programs and couldn't ignore it anymore.

I went through all of my loaded assets (textures, sounds, images) and everything seemed to line up. What did it in the end was adding a compiler flag to sanitize addresses.

`-fsanitize=address`

This gave me an output which told me the I was getting stack overflow when accessing the obstacles array. Something I hadn't touched in a long time and assumed was sound code. I was taking the lenth of the array as the maxiumum index, so every time I would loop through the obstales (multiple times every frame) it would overflow.

Changing these fixed the errors, very happy to see it.


