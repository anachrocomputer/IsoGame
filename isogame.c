/* isogame --- isometric game in SDL                        2016-11-20 */

#include <SDL2/SDL.h> 
#include <stdio.h> 

/* Window size */
#define SCREEN_WIDTH   (640) 
#define SCREEN_HEIGHT  (480)

void isoSetPixel(SDL_Surface *screenSurface, const int x, const int y, const int z, const Uint32 colour);
void isoDrawCube(SDL_Surface *screenSurface, const int hlx, const int hly, const int hlz,
                 const Uint32 red, const Uint32 yellow, const Uint32 green);
void isoDrawBlock(SDL_Surface *screenSurface, const int x, const int y, const int z, const SDL_Surface *img);
void isoDrawAnimatedBlock(SDL_Surface *screenSurface, const int x, const int y, const int z, const SDL_Surface *img, const int frame);

int main(int argc, char *argv[])
{
   SDL_Joystick *joystick = NULL;   /* The SDL joystick */
   SDL_Window *window = NULL;       /* The SDL window */

   /* The surface contained by the window */
   SDL_Surface *screenSurface = NULL;

   SDL_Surface *tempImage = NULL;
   SDL_Surface *pixelImage = NULL;
   SDL_Surface *cubeImage = NULL;
   Uint32 white, blue, black, grey; /* Precomputed colours */
   Uint32 red, yellow, green;       /* Precomputed colours */
   Uint32 transparent;
   int nb;
   int jb[16];
   SDL_Rect jbut[16];
   SDL_Rect src, dest;
   int frame = 0;
   short int phaseAcc = 0;
   int na;
   int ja[16];
   SDL_Rect jaxis[16], jposn[16];
   int nh;
   SDL_Event e;
   int running = 1;
   int i, x0, y0, z0;
   int hlx, hly, hlz;

   hlx = 0;
   hly = 0;
   hlz = 0;
      
   /* Initialise Simple DirectMedia Layer */
   if (SDL_Init(SDL_INIT_EVERYTHING)) {
      printf("SDL failed to initialise: SDL_Error: %s\n", SDL_GetError());
      exit(EXIT_FAILURE);
   }                    

   for (i = 0; i < 16; i++) {
      ja[i] = 0;
      jaxis[i].x = 10 + (16 * i);
      jaxis[i].y = 30;
      jaxis[i].w = 10;
      jaxis[i].h = 128 + 10;
      
      jposn[i].x = jaxis[i].x;
      jposn[i].y = jaxis[i].y;
      jposn[i].w = jaxis[i].w;
      jposn[i].h = 10;
   }
   
   for (i = 0; i < 16; i++) {
      jb[i] = 0;
      jbut[i].x = 10 + (16 * i);
      jbut[i].y = 10;
      jbut[i].w = 10;
      jbut[i].h = 10;
   }
   
   /* Read in an image and make it the same format as our display */
   if ((pixelImage = SDL_LoadBMP("tile.bmp")) == NULL) {
      printf("Error: could not load 'tile.bmp': %s.\n", SDL_GetError());
   }
   
// pixelImage = SDL_DisplayFormat(tempImage);
// SDL_FreeSurface(tempImage);
   printf("Size of 'tile.bmp' is %dx%d\n", pixelImage->w, pixelImage->h);
   
   /* Read in another image and make it the same format as our display */
   if ((cubeImage = SDL_LoadBMP("cube.bmp")) == NULL) {
      printf("Error: could not load 'cube.bmp': %s.\n", SDL_GetError());
   }

   printf("Size of 'cube.bmp' is %dx%d\n", cubeImage->w, cubeImage->h);
   
   /* Do we have any joysticks connected? */
   if (SDL_NumJoysticks() < 1) {
      printf("Warning: No joysticks connected\n");
      na = nb = nh = 0;
   }
   else { 
      /* If we have one, open joystick */
      joystick = SDL_JoystickOpen(0);
      if (joystick == NULL) { 
         printf("Warning: Unable to open game controller: SDL Error: %s\n", SDL_GetError());
         na = nb = nh = 0;
      }
      else {
#ifdef DB
         printf("Name: %s\n", SDL_JoystickNameForIndex(0));
         printf("Number of Axes: %d\n", SDL_JoystickNumAxes(joystick));
         printf("Number of Buttons: %d\n", SDL_JoystickNumButtons(joystick));
         printf("Number of Hats: %d\n", SDL_JoystickNumHats(joystick));
#endif
         
         na = SDL_JoystickNumAxes(joystick);
         nb = SDL_JoystickNumButtons(joystick);
         nh = SDL_JoystickNumHats(joystick);
      }
   }

   /* Create SDL window */
   window = SDL_CreateWindow("IsoGame", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);

   if (window == NULL) { 
      printf("Window could not be created: SDL_Error: %s\n", SDL_GetError());
      SDL_Quit();
      exit (EXIT_FAILURE);
   }               

   /* Get window surface */
   screenSurface = SDL_GetWindowSurface(window); 

   /* Generate some colours */
   white  = SDL_MapRGB(screenSurface->format, 0xff, 0xff, 0xff);
   blue   = SDL_MapRGB(screenSurface->format, 0x40, 0x40, 0xff);
   black  = SDL_MapRGB(screenSurface->format, 0x00, 0x00, 0x00);
   grey   = SDL_MapRGB(screenSurface->format, 0x80, 0x80, 0x80);
   red    = SDL_MapRGB(screenSurface->format, 0xff, 0x40, 0x40);
   yellow = SDL_MapRGB(screenSurface->format, 0xff, 0xff, 0x40);
   green  = SDL_MapRGB(screenSurface->format, 0x40, 0xff, 0x40);
   
   transparent = SDL_MapRGB(pixelImage->format, 0xff, 0x00, 0xff);
   
   if (SDL_SetColorKey(pixelImage, SDL_TRUE, transparent) < 0) {
      printf("SDL_SetColorKey failed: %s\n", SDL_GetError());
   }
   
   if (SDL_SetColorKey(cubeImage, SDL_TRUE, transparent) < 0) {
      printf("SDL_SetColorKey failed: %s\n", SDL_GetError());
   }
   
   /* Main loop */
   do {
      while (SDL_PollEvent(&e) != 0) {
#ifdef DB
         printf("EVENT: %d ", e.type);
#endif

         if (e.type == SDL_QUIT) {
#ifdef DB
            printf("QUIT\n");
#endif
            running = 0;
         }
         else if (e.type == SDL_WINDOWEVENT) {
#ifdef DB
            printf("WINDOW EVENT %d\n", e.window.event);
#endif
         }
         else if (e.type == SDL_MOUSEMOTION) {
#ifdef DB
            printf("MOUSE %d TO %d,%d\n", e.motion.which, e.motion.x, e.motion.y);
#endif
         }
         else if (e.type == SDL_MOUSEBUTTONDOWN) {
#ifdef DB
            printf("MOUSE %d BUTTON %d DOWN\n", e.button.which, e.button.button);
#endif
         }
         else if (e.type == SDL_MOUSEBUTTONUP) {
#ifdef DB
            printf("MOUSE %d BUTTON %d UP\n", e.button.which, e.button.button);
#endif
         }
         else if (e.type == SDL_JOYAXISMOTION) {
#ifdef DB
            printf("JOYSTICK %d MOVED AXIS %d TO %d\n", e.jaxis.which, e.jaxis.axis, e.jaxis.value);
#endif
            if (e.jaxis.which == 0) {
               ja[e.jaxis.axis] = e.jaxis.value;
               
               switch (e.jaxis.axis) {
               case 0:
                  if (e.jaxis.value < -8000) {
                     if (hlx > 0)
                        hlx--;
                  }
                  else if (e.jaxis.value > 8000) {
                     if (hlx < 7)
                        hlx++;
                  }
                  break;
               case 1:
                  if (e.jaxis.value < -8000) {
                     if (hly < 7)
                        hly++;
                  }
                  else if (e.jaxis.value > 8000) {
                     if (hly > 0)
                        hly--;
                  }
                  break;
               case 2:
                  break;
               case 3:
                  break;
               }
            }
         }
         else if (e.type == SDL_JOYHATMOTION) {
#ifdef DB
            printf("JOYSTICK %d HAT %d TO %d\n", e.jhat.which, e.jhat.hat, e.jhat.value);
#endif
         }
         else if (e.type == SDL_JOYBUTTONDOWN) {
#ifdef DB
            printf("JOYSTICK %d BUTTON %d DOWN\n", e.jbutton.which, e.jbutton.button);
#endif
            jb[e.jbutton.button] = 1;
            switch (e.jbutton.button) {
            case 0:
               if (hlx < 7)
                  hlx++;
               
               if (hly < 7)
                  hly++;

               break;
            case 1:
               if (hlx < 7)
                  hlx++;
               
               if (hly > 0)
                  hly--;
                  
               break;
            case 2:
               if (hlx > 0)
                  hlx--;
                  
               if (hly > 0)
                  hly--;
                  
               break;
            case 3:
               if (hlx > 0)
                  hlx--;

               if (hly < 7)
                  hly++;
                  
               break;
            case 4:
               break;
            case 8:
               running = 0;
               break;
            }
         }
         else if (e.type == SDL_JOYBUTTONUP) {
#ifdef DB
            printf("JOYSTICK %d BUTTON %d UP\n", e.jbutton.which, e.jbutton.button);
#endif
            jb[e.jbutton.button] = 0;
         }
         else if (e.type == SDL_KEYDOWN) {
#ifdef DB
            printf("KEYBOARD %d DOWN\n", e.key.keysym.sym);
#endif
            
            switch (e.key.keysym.sym) {
            case SDLK_UP:
               if (hly < 7)
                  hly++;

               break;
            case SDLK_DOWN:
               if (hly > 0)
                  hly--;

               break;
            case SDLK_LEFT:
               if (hlx > 0)
                  hlx--;
                  
               break;
            case SDLK_RIGHT:
               if (hlx < 7)
                  hlx++;
                  
               break;
            case SDLK_q:
               running = 0;
               break;
            }
         }
         else if (e.type == SDL_KEYUP) {
#ifdef DB
            printf("KEYBOARD %d UP\n", e.key.keysym.sym);
#endif
         }
         else {
#ifdef DB
            printf("UNHANDLED\n");
#endif
         }
      }
      
      /* Fill the surface white */
      SDL_FillRect(screenSurface, NULL, white);
      
      /* Display joystick buttons */
      for (i = 0; i < 16; i++) {
         if (i < nb) {
            if (jb[i])
               SDL_FillRect(screenSurface, &jbut[i], blue);
            else
               SDL_FillRect(screenSurface, &jbut[i], black);
         }
         else
            SDL_FillRect(screenSurface, &jbut[i], grey);
      }
      
      for (i = 0; i < 16; i++) {
         if (i < na) {
            SDL_FillRect(screenSurface, &jaxis[i], black);
            jposn[i].y = jaxis[i].y + (ja[i] / 512) + 64;
            SDL_FillRect(screenSurface, &jposn[i], blue);
         }
         else
            SDL_FillRect(screenSurface, &jaxis[i], grey);
      }
      
      /* Draw an isometric floor grid */
      for (y0 = 0; y0 <= 320; y0 += 40) {
         for (x0 = 0; x0 <= 320; x0 += 2) {
            isoSetPixel(screenSurface, x0, y0, 0, black);
         }
      }

      for (x0 = 0; x0 <= 320; x0 += 40) {
         for (y0 = 0; y0 <= 320; y0 += 2) {
            isoSetPixel(screenSurface, x0, y0, 0, black);
         }
      }

      /* Draw an isometric wall grid on the x=8 plane */
#if 0
      for (y0 = 0; y0 <= 320; y0 += 40) {
         for (z0 = 0; z0 <= 160; z0 += 2) {
            isoSetPixel(screenSurface, 320, y0, z0, black);
         }
      }

      for (z0 = 0; z0 <= 160; z0 += 40) {
         for (y0 = 0; y0 <= 320; y0 += 2) {
            isoSetPixel(screenSurface, 320, y0, z0, black);
         }
      }
#endif
      
      /* Draw an isometric wall grid on the y=8 plane */
#if 0
      for (x0 = 0; x0 <= 320; x0 += 40) {
         for (z0 = 0; z0 <= 160; z0 += 2) {
            isoSetPixel(screenSurface, x0, 320, z0, black);
         }
      }

      for (z0 = 0; z0 <= 160; z0 += 40) {
         for (x0 = 0; x0 <= 320; x0 += 2) {
            isoSetPixel(screenSurface, x0, 320, z0, black);
         }
      }
#endif
      
      /* Draw the left-hand rear wall */
      for (z0 = 0; z0 < 4; z0++) {
         isoDrawBlock(screenSurface, 7, 8, z0, cubeImage);
         isoDrawBlock(screenSurface, 6, 8, z0, cubeImage);
         isoDrawBlock(screenSurface, 5, 8, z0, cubeImage);
         
         if (z0 == 3) {
            isoDrawBlock(screenSurface, 4, 8, z0, cubeImage);
            isoDrawBlock(screenSurface, 3, 8, z0, cubeImage);
         }
         
         isoDrawBlock(screenSurface, 2, 8, z0, cubeImage);
         isoDrawBlock(screenSurface, 1, 8, z0, cubeImage);
         isoDrawBlock(screenSurface, 0, 8, z0, cubeImage);
      }
      
      /* Draw the right-hand rear wall */
      for (z0 = 0; z0 < 4; z0++) {
         isoDrawBlock(screenSurface, 8, 7, z0, cubeImage);
         isoDrawBlock(screenSurface, 8, 6, z0, cubeImage);
         isoDrawBlock(screenSurface, 8, 5, z0, cubeImage);
         
         if (z0 == 3) {
            isoDrawBlock(screenSurface, 8, 4, z0, cubeImage);
            isoDrawBlock(screenSurface, 8, 3, z0, cubeImage);
         }
         
         isoDrawBlock(screenSurface, 8, 2, z0, cubeImage);
         isoDrawBlock(screenSurface, 8, 1, z0, cubeImage);
         isoDrawBlock(screenSurface, 8, 0, z0, cubeImage);
      }
      
      /* Highlight one square on the isometric grid */
      isoDrawCube(screenSurface, hlx, hly, hlz, red, yellow, green);

      frame = (phaseAcc >> 8) & 0x07;
      phaseAcc += 16;
      
      /* Draw some animated and static images */
      isoDrawBlock(screenSurface, 6, 6, 0, cubeImage);
      isoDrawBlock(screenSurface, 6, 5, 0, cubeImage);
      isoDrawBlock(screenSurface, 6, 4, 0, cubeImage);
      
      isoDrawAnimatedBlock(screenSurface, 6, 5, 1, cubeImage, frame);
      
      /* Draw an animated image */
      src.x = 128 * frame;
      src.y = 0;
      src.w = 128;
      src.h = pixelImage->h;
      
      dest.x = 0;
      dest.y = SCREEN_HEIGHT - pixelImage->h;
      
      SDL_BlitSurface(pixelImage, &src, screenSurface, &dest);

      /* Update the surface */
      SDL_UpdateWindowSurface(window);

      SDL_Delay(40);
   } while (running);
   
   /* Destroy bitmaps */
   SDL_FreeSurface(pixelImage);
   
   /* Destroy window */
   SDL_DestroyWindow(window);

   SDL_JoystickClose(joystick);
   
   /* Quit SDL subsystem */
   SDL_Quit();

   return (EXIT_SUCCESS);
}

void isoSetPixel(SDL_Surface *screenSurface, const int x, const int y, const int z, const Uint32 colour)
{
   SDL_Rect pixel;

   pixel.x = ((SCREEN_WIDTH / 2) + x) - (y / 1);
   pixel.y = ((SCREEN_HEIGHT - 1) - (x / 2)) - (y / 2) - z;
   pixel.w = 2;
   pixel.h = 2;

   SDL_FillRect(screenSurface, &pixel, colour);
}


void isoDrawCube(SDL_Surface *screenSurface, const int hlx, const int hly, const int hlz,
                 const Uint32 top, const Uint32 left, const Uint32 right)
{
   int x, y, z;
    
   z = 40;
   for (x = (hlx * 40) + 2; x < (hlx * 40) + 40; x += 2) {
      for (y = (hly * 40) + 2; y < (hly * 40) + 40; y += 2) {
         isoSetPixel(screenSurface, x, y, z, top);
      }
   }

   x = (hlx * 40) + 2;
   for (z = (hlz * 40) + 0; z < (hlz * 40) + 38; z += 2) {
      for (y = (hly * 40) + 4; y < (hly * 40) + 42; y += 2) {
         isoSetPixel(screenSurface, x, y, z, left);
      }
   }

   y = (hly * 40) + 2;
   for (z = (hlz * 40) + 0; z < (hlz * 40) + 38; z += 2) {
      for (x = (hlx * 40) + 4; x < (hlx * 40) + 42; x += 2) {
         isoSetPixel(screenSurface, x, y, z, right);
      }
   }
}


void isoDrawBlock(SDL_Surface *screenSurface, const int x, const int y, const int z, const SDL_Surface *img)
{
   SDL_Rect src;
   SDL_Rect dest;
   int x0, y0, z0;

   x0 = x * 40;
   y0 = y * 40;
   z0 = z * 40;

   src.x = 0;
   src.y = 0;
   src.w = 80;
   src.h = img->h;

   dest.x = ((SCREEN_WIDTH / 2) + x0) - (y0 / 1) - 40;
   dest.y = ((SCREEN_HEIGHT - 1) - (x0 / 2)) - (y0 / 2) - z0 - 80;

   SDL_BlitSurface(img, &src, screenSurface, &dest);
}


void isoDrawAnimatedBlock(SDL_Surface *screenSurface, const int x, const int y, const int z, const SDL_Surface *img, const int frame)
{
   SDL_Rect src;
   SDL_Rect dest;
   int x0, y0, z0;

   x0 = x * 40;
   y0 = y * 40;
   z0 = z * 40;

   src.x = frame * 80;
   src.y = 0;
   src.w = 80;
   src.h = img->h;

   dest.x = ((SCREEN_WIDTH / 2) + x0) - (y0 / 1) - 40;
   dest.y = ((SCREEN_HEIGHT - 1) - (x0 / 2)) - (y0 / 2) - z0 - 80;

   SDL_BlitSurface(img, &src, screenSurface, &dest);
}

