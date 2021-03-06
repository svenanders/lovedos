/** 
 * Copyright (c) 2014 rxi
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the MIT license. See LICENSE for details.
 */

#include <string.h>
#include <stdlib.h>
#include "luaobj.h"
#include "image.h"
#include "font.h"
#include "quad.h"
#include "vga.h"
#include "luaobj.h"

image_t  *graphics_screen;
font_t   *graphics_defaultFont;

image_t  *graphics_canvas;
font_t   *graphics_font;
pixel_t   graphics_backgroundColor;
pixel_t   graphics_color;
int       graphics_blendMode;
int       graphics_flip;



int l_graphics_getDimensions(lua_State *L) {
  lua_pushinteger(L, graphics_screen->width);
  lua_pushinteger(L, graphics_screen->height);
  return 2;
}


int l_graphics_getWidth(lua_State *L) {
  lua_pushinteger(L, graphics_screen->width);
  return 1;
}


int l_graphics_getHeight(lua_State *L) {
  lua_pushinteger(L, graphics_screen->height);
  return 1;
}


int l_graphics_getBackgroundColor(lua_State *L) {
  lua_pushinteger(L, graphics_backgroundColor);
  return 1;
}


int l_graphics_setBackgroundColor(lua_State *L) {
  graphics_backgroundColor =
    lua_isnoneornil(L, 1) ? 0x0: luaL_checkint(L, 1);
  return 0;
}


int l_graphics_getColor(lua_State *L) {
  lua_pushinteger(L, graphics_backgroundColor);
  return 1;
}


int l_graphics_setColor(lua_State *L) {
  graphics_color = lua_isnoneornil(L, 1) ? 0xf : luaL_checkint(L, 1);
  image_setColor(graphics_color);
  return 0;
}


int l_graphics_getBlendMode(lua_State *L) {
  switch (graphics_blendMode) {
    default:
    case IMAGE_NORMAL : lua_pushstring(L, "normal");  break;
    case IMAGE_FAST   : lua_pushstring(L, "fast");    break;
    case IMAGE_AND    : lua_pushstring(L, "and");     break;
    case IMAGE_OR     : lua_pushstring(L, "or");      break;
    case IMAGE_COLOR  : lua_pushstring(L, "color");   break;
  }
  return 1;
}


int l_graphics_setBlendMode(lua_State *L) {
  const char *str = lua_isnoneornil(L, 1) ? "normal" : luaL_checkstring(L, 1);
  #define SET_BLEND_MODE(str, e)\
    do {\
      if (!strcmp(str, str)) {\
        graphics_blendMode = e;\
        image_setBlendMode(graphics_blendMode);\
        return 0;\
      }\
    } while (0)

  switch (*str) {
    case 'n'  : SET_BLEND_MODE("normal",  IMAGE_NORMAL);  break;
    case 'f'  : SET_BLEND_MODE("fast",    IMAGE_FAST);    break;
    case 'a'  : SET_BLEND_MODE("and",     IMAGE_AND);     break;
    case 'o'  : SET_BLEND_MODE("or",      IMAGE_OR);      break;
    case 'c'  : SET_BLEND_MODE("color",   IMAGE_COLOR);   break;
  }
  #undef SET_BLEND_MODE
  luaL_argerror(L, 1, "bad blend mode");
  return 0;
}


int l_graphics_getFont(lua_State *L) {
  lua_pushlightuserdata(L, graphics_font);
  lua_gettable(L, LUA_REGISTRYINDEX);
  return 1;
}


int l_graphics_setFont(lua_State *L) {
  font_t *oldFont = graphics_font;
  if (lua_isnoneornil(L, 1)) {
    /* If no arguments are given we use the default embedded font, grab it
     * from the registry and set it as the first argument */
    lua_pushlightuserdata(L, &graphics_defaultFont);
    lua_gettable(L, LUA_REGISTRYINDEX);
    lua_insert(L, 1);
  }
  graphics_font = luaobj_checkudata(L, 1, LUAOBJ_TYPE_FONT);
  /* Remove old font from registry. This is done after we know the args are
   * okay so that the font remains unchanged if an error occurs */
  if (oldFont) {
    lua_pushlightuserdata(L, oldFont);
    lua_pushnil(L);
    lua_settable(L, LUA_REGISTRYINDEX);
  }
  /* Add new font to registry */
  lua_pushlightuserdata(L, graphics_font);
  lua_pushvalue(L, 1);
  lua_settable(L, LUA_REGISTRYINDEX);
  return 0;
}


int l_graphics_getCanvas(lua_State *L) {
  lua_pushlightuserdata(L, graphics_canvas);
  lua_gettable(L, LUA_REGISTRYINDEX);
  return 1;
}


int l_graphics_setCanvas(lua_State *L) {
  image_t *oldCanvas = graphics_canvas;
  if (lua_isnoneornil(L, 1)) {
    /* If no arguments are given we use the screen canvas, grab it from the
     * registry and set it as the first argument */
    lua_pushlightuserdata(L, &graphics_screen);
    lua_gettable(L, LUA_REGISTRYINDEX);
    lua_insert(L, 1);
  } 
  graphics_canvas = luaobj_checkudata(L, 1, LUAOBJ_TYPE_IMAGE);
  /* Remove old canvas from registry. This is done after we know the args are
   * okay so that the canvas remains unchanged if an error occurs */
  if (oldCanvas) {
    lua_pushlightuserdata(L, oldCanvas);
    lua_pushnil(L);
    lua_settable(L, LUA_REGISTRYINDEX);
  }
  /* Add new canvas to registry */
  lua_pushlightuserdata(L, graphics_canvas);
  lua_pushvalue(L, 1);
  lua_settable(L, LUA_REGISTRYINDEX);
  return 0;
}


int l_graphics_getFlip(lua_State *L) {
  lua_pushboolean(L, graphics_flip);
  return 1;
}


int l_graphics_setFlip(lua_State *L) {
  int x = lua_isnoneornil(L, 1) ? 0 : lua_toboolean(L, 1);
  image_setFlip(x);
  return 0;
}


int l_graphics_reset(lua_State *L) {
  int (*funcs[])(lua_State*) = {
    l_graphics_setBackgroundColor,
    l_graphics_setColor,
    l_graphics_setBlendMode,
    l_graphics_setFont,
    l_graphics_setCanvas,
    l_graphics_setFlip,
    NULL,
  };
  int i;
  for (i = 0; funcs[i]; i++) {
    lua_pushcfunction(L, funcs[i]);
    lua_call(L, 0, 0);
  }
  return 0;
}


int l_graphics_clear(lua_State *L) {
  int color = lua_isnoneornil(L, 1) ? graphics_backgroundColor
                                    : luaL_checkint(L, 1);
  memset(graphics_canvas->data, color,
         graphics_canvas->width * graphics_canvas->height);
  return 0;
}


int l_graphics_present(lua_State *L) {
  vga_update(graphics_screen->data);
  return 0;
}


int l_graphics_draw(lua_State *L) {
  image_t *img = luaobj_checkudata(L, 1, LUAOBJ_TYPE_IMAGE);
  quad_t *quad = NULL;
  int x, y;
  if (!lua_isnone(L, 2) && lua_type(L, 2) != LUA_TNUMBER) {
    quad = luaobj_checkudata(L, 2, LUAOBJ_TYPE_QUAD);
    x = luaL_checkint(L, 3);
    y = luaL_checkint(L, 4);
  } else {
    x = luaL_checkint(L, 2);
    y = luaL_checkint(L, 3);
  }
  pixel_t *buf = graphics_canvas->data;
  int bufw = graphics_canvas->width;
  int bufh = graphics_canvas->height;
  if (quad) {
    image_blit(img, buf, bufw, bufh, x, y,
               quad->x, quad->y, quad->width, quad->height);
  } else {
    image_blit(img, buf, bufw, bufh, x, y,
               0, 0, img->width, img->height);
  }
  return 0;
}


int l_graphics_point(lua_State *L) {
  int x = luaL_checkint(L, 1);
  int y = luaL_checkint(L, 2);
  image_setPixel(graphics_canvas, x, y, graphics_color);
  return 0;
}


int l_graphics_line(lua_State *L) {
  int argc = lua_gettop(L);
  int lastx = luaL_checkint(L, 1);
  int lasty = luaL_checkint(L, 2);
  int idx = 3;
  while (idx < argc) {
    int x0 = lastx;
    int y0 = lasty;
    int x1 = luaL_checkint(L, idx);
    int y1 = luaL_checkint(L, idx + 1);
    lastx = x1;
    lasty = y1;
    /* Draw line */
    #define SWAP_INT(a, b) (((a) ^= (b)), ((b) ^= (a)), ((a) ^= (b)))
    int steep = abs(y1 - y0) > abs(x1 - x0);
    if (steep) {
      SWAP_INT(x0, y0);
      SWAP_INT(x1, y1);
    }
    if (x0 > x1) {
      SWAP_INT(x0, x1);
      SWAP_INT(y0, y1);
    }
    #undef SWAP_INT
    int deltax = x1 - x0;
    int deltay = abs(y1 - y0);
    int error = deltax / 2;
    int ystep = (y0 < y1) ? 1 : -1;
    int x, y = y0;
    for (x = x0; x < x1; x++) {
      if (steep) {
        image_setPixel(graphics_canvas, y, x, graphics_color);
      } else {
        image_setPixel(graphics_canvas, x, y, graphics_color);
      }
      error -= deltay;
      if (error < 0) {
        y += ystep;
        error += deltax;
      }
    }
    idx += 2;
  }
  return 0;
}


int l_graphics_rectangle(lua_State *L) {
  const char *mode = luaL_checkstring(L, 1);
  int x = luaL_checkint(L, 2);
  int y = luaL_checkint(L, 3);
  int x2 = luaL_checkint(L, 4) + x;
  int y2 = luaL_checkint(L, 5) + y;
  int fill = 0;
  if (!strcmp(mode, "fill")) {
    fill = 1;
  } else if (!strcmp(mode, "line")) {
    fill = 0;
  } else {
    luaL_error(L, "bad mode");
  }
  /* Clip to screen */
  if (x < 0) { x2 += x; x = 0; }
  if (y < 0) { y2 += y; y = 0; }
  if (x2 > graphics_canvas->width)  { x2 = graphics_canvas->width; }
  if (y2 > graphics_canvas->height) { y2 = graphics_canvas->height; }
  /* Get width/height and Abort early if we're off screen */
  int width = x2 - x;
  int height = y2 - y;
  if (width <= 0 || height <= 0) return 0;
  /* Draw */
  if (fill) {
    int i;
    for (i = y; i < y2; i++) {
      memset(graphics_canvas->data + x + i * graphics_canvas->width,
             graphics_color, width);
    }
  } else {
      memset(graphics_canvas->data + x + y * graphics_canvas->width,
             graphics_color, width);
      memset(graphics_canvas->data + x + (y2 - 1) * graphics_canvas->width,
             graphics_color, width);
      int i;
      for (i = y; i < y2; i++) {
        graphics_canvas->data[x + i * graphics_canvas->width] =
          graphics_canvas->data[x2 - 1 + i * graphics_canvas->width] =
            graphics_color;
      }
  }
  return 0;
}


int l_graphics_circle(lua_State *L) {
  const char *mode = luaL_checkstring(L, 1);
  int x = luaL_checkint(L, 2);
  int y = luaL_checkint(L, 3);
  int radius = luaL_checkint(L, 4);
  int fill = 0;
  if (!strcmp(mode, "fill")) {
    fill = 1;
  } else if (!strcmp(mode, "line")) {
    fill = 0;
  } else {
    luaL_error(L, "bad mode");
  }
  /* Draw */
  if (fill) {
    int dx = radius, dy = 0;
    int radiusError = 1-dx;
    while(dx >= dy) {
      #define FILL_ROW(startx, endx, starty)\
        do {\
          int sx = (startx);\
          int ex = (endx);\
          int sy = (starty);\
          if (sy < 0 || sy >= graphics_canvas->height) break;\
          if (sx < 0) sx = 0;\
          if (sx >= graphics_canvas->width) sx = graphics_canvas->width - 1;\
          if (ex < 0) ex = 0;\
          if (ex > graphics_canvas->width) ex = graphics_canvas->width;\
          if (sx == ex) break;\
          memset(graphics_canvas->data + sx + sy * graphics_canvas->width,\
                 graphics_color, ex - sx);\
        } while (0)

      FILL_ROW( -dx + x,  dx + x,   dy + y );
      FILL_ROW( -dx + x,  dx + x,  -dy + y );
      FILL_ROW( -dy + x,  dy + x,   dx + y );
      FILL_ROW( -dy + x,  dy + x,  -dx + y );

      #undef FILL_ROW
      dy++;
      if(radiusError<0) {
        radiusError+=2*dy+1;
      } else {
        dx--;
        radiusError+=2*(dy-dx+1);
      }
    }
  } else {
    int dx = radius, dy = 0;
    int radiusError = 1-dx;
    while(dx >= dy) {
      image_setPixel(graphics_canvas,  dx + x,   dy + y, graphics_color);
      image_setPixel(graphics_canvas, -dx + x,   dy + y, graphics_color);
      image_setPixel(graphics_canvas,  dx + x,  -dy + y, graphics_color);
      image_setPixel(graphics_canvas, -dx + x,  -dy + y, graphics_color);
      image_setPixel(graphics_canvas,  dy + x,   dx + y, graphics_color);
      image_setPixel(graphics_canvas, -dy + x,   dx + y, graphics_color);
      image_setPixel(graphics_canvas,  dy + x,  -dx + y, graphics_color);
      image_setPixel(graphics_canvas, -dy + x,  -dx + y, graphics_color);
      dy++;
      if(radiusError<0) {
        radiusError+=2*dy+1;
      } else {
        dx--;
        radiusError+=2*(dy-dx+1);
      }
    }
  }
  return 0;
}


int l_graphics_print(lua_State *L) {
  luaL_checkany(L, 1);
  const char *str = luaL_tolstring(L, 1, NULL);
  int x = luaL_checkint(L, 2);
  int y = luaL_checkint(L, 3);
  font_blit(graphics_font, graphics_canvas->data, graphics_canvas->width,
            graphics_canvas->height, str, x, y);
  return 0;
}



int l_image_new(lua_State *L);
int l_image_newCanvas(lua_State *L);
int l_quad_new(lua_State *L);
int l_font_new(lua_State *L);

int luaopen_graphics(lua_State *L) {
  luaL_Reg reg[] = {
    { "getDimensions",      l_graphics_getDimensions      },
    { "getWidth",           l_graphics_getWidth           },
    { "getHeight",          l_graphics_getHeight          },
    { "getBackgroundColor", l_graphics_getBackgroundColor },
    { "setBackgroundColor", l_graphics_setBackgroundColor },
    { "getColor",           l_graphics_getColor           },
    { "setColor",           l_graphics_setColor           },
    { "getBlendMode",       l_graphics_getBlendMode       },
    { "setBlendMode",       l_graphics_setBlendMode       },
    { "getFont",            l_graphics_getFont            },
    { "setFont",            l_graphics_setFont            },
    { "getCanvas",          l_graphics_getCanvas          },
    { "setCanvas",          l_graphics_setCanvas          },
    { "getFlip",            l_graphics_getFlip            },
    { "setFlip",            l_graphics_setFlip            },
    { "reset",              l_graphics_reset              },
    { "clear",              l_graphics_clear              },
    { "present",            l_graphics_present            },
    { "draw",               l_graphics_draw               },
    { "point",              l_graphics_point              },
    { "line",               l_graphics_line               },
    { "rectangle",          l_graphics_rectangle          },
    { "circle",             l_graphics_circle             },
    { "print",              l_graphics_print              },
    { "newImage",           l_image_new                   },
    { "newCanvas",          l_image_newCanvas             },
    { "newQuad",            l_quad_new                    },
    { "newFont",            l_font_new                    },
    { 0, 0 },
  };
  luaL_newlib(L, reg);

  /* Init screen canvas */
  lua_pushcfunction(L, l_image_newCanvas);
  lua_pushinteger(L, VGA_WIDTH);
  lua_pushinteger(L, VGA_HEIGHT);
  lua_call(L, 2, 1);
  graphics_screen = luaobj_checkudata(L, -1, LUAOBJ_TYPE_IMAGE);
  /* Add screen canvas to registry */
  lua_pushlightuserdata(L, &graphics_screen);
  lua_pushvalue(L, -2);
  lua_settable(L, LUA_REGISTRYINDEX);
  lua_pop(L, 1); /* Pop the Image object */

  /* Init default font */
  lua_pushcfunction(L, l_font_new);
  lua_call(L, 0, 1);
  graphics_defaultFont = luaobj_checkudata(L, -1, LUAOBJ_TYPE_FONT);
  /* Add default font to registry */
  lua_pushlightuserdata(L, &graphics_defaultFont);
  lua_pushvalue(L, -2);
  lua_settable(L, LUA_REGISTRYINDEX);
  lua_pop(L, 1); /* Pop the Font object */

  /* Reset all state settings to their defaults */
  lua_pushcfunction(L, l_graphics_reset);
  lua_call(L, 0, 0);

  return 1;
}
