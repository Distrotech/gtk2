/* gdkkeys-quartz.c
 *
 * Copyright (C) 2000 Red Hat, Inc.
 * Copyright (C) 2005 Imendio AB
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */
/* Some parts of this code come from quartzKeyboard.c,
 * from the Apple X11 Server.
 *
 * Copyright (c) 2003 Apple Computer, Inc. All rights reserved.
 *
 *  Permission is hereby granted, free of charge, to any person
 *  obtaining a copy of this software and associated documentation files
 *  (the "Software"), to deal in the Software without restriction,
 *  including without limitation the rights to use, copy, modify, merge,
 *  publish, distribute, sublicense, and/or sell copies of the Software,
 *  and to permit persons to whom the Software is furnished to do so,
 *  subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be
 *  included in all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 *  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 *  NONINFRINGEMENT.  IN NO EVENT SHALL THE ABOVE LISTED COPYRIGHT
 *  HOLDER(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 *  WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 *  DEALINGS IN THE SOFTWARE.
 *
 *  Except as contained in this notice, the name(s) of the above
 *  copyright holders shall not be used in advertising or otherwise to
 *  promote the sale, use or other dealings in this Software without
 *  prior written authorization.
 */

#include "config.h"

#include <Carbon/Carbon.h>
#include <AppKit/NSEvent.h>
#include "gdk.h"
#include "gdkkeysyms.h"

#define NUM_KEYCODES 128
#define KEYVALS_PER_KEYCODE 4

static GdkKeymap *default_keymap = NULL;

/* Note: we could check only if building against the 10.5 SDK instead, but
 * that would make non-xml layouts not work in 32-bit which would be a quite
 * bad regression. This way, old unsupported layouts will just not work in
 * 64-bit.
 */
#ifdef __LP64__
static TISInputSourceRef current_layout = NULL;
#else
static KeyboardLayoutRef current_layout = NULL;
#endif

/* This is a table of all keyvals. Each keycode gets KEYVALS_PER_KEYCODE entries.
 * TThere is 1 keyval per modifier (Nothing, Shift, Alt, Shift+Alt);
 */
static guint *keyval_array = NULL;

static inline UniChar
macroman2ucs (unsigned char c)
{
  /* Precalculated table mapping MacRoman-128 to Unicode. Generated
     by creating single element CFStringRefs then extracting the
     first character. */
  
  static const unsigned short table[128] = {
    0xc4, 0xc5, 0xc7, 0xc9, 0xd1, 0xd6, 0xdc, 0xe1,
    0xe0, 0xe2, 0xe4, 0xe3, 0xe5, 0xe7, 0xe9, 0xe8,
    0xea, 0xeb, 0xed, 0xec, 0xee, 0xef, 0xf1, 0xf3,
    0xf2, 0xf4, 0xf6, 0xf5, 0xfa, 0xf9, 0xfb, 0xfc,
    0x2020, 0xb0, 0xa2, 0xa3, 0xa7, 0x2022, 0xb6, 0xdf,
    0xae, 0xa9, 0x2122, 0xb4, 0xa8, 0x2260, 0xc6, 0xd8,
    0x221e, 0xb1, 0x2264, 0x2265, 0xa5, 0xb5, 0x2202, 0x2211,
    0x220f, 0x3c0, 0x222b, 0xaa, 0xba, 0x3a9, 0xe6, 0xf8,
    0xbf, 0xa1, 0xac, 0x221a, 0x192, 0x2248, 0x2206, 0xab,
    0xbb, 0x2026, 0xa0, 0xc0, 0xc3, 0xd5, 0x152, 0x153,
    0x2013, 0x2014, 0x201c, 0x201d, 0x2018, 0x2019, 0xf7, 0x25ca,
    0xff, 0x178, 0x2044, 0x20ac, 0x2039, 0x203a, 0xfb01, 0xfb02,
    0x2021, 0xb7, 0x201a, 0x201e, 0x2030, 0xc2, 0xca, 0xc1,
    0xcb, 0xc8, 0xcd, 0xce, 0xcf, 0xcc, 0xd3, 0xd4,
    0xf8ff, 0xd2, 0xda, 0xdb, 0xd9, 0x131, 0x2c6, 0x2dc,
    0xaf, 0x2d8, 0x2d9, 0x2da, 0xb8, 0x2dd, 0x2db, 0x2c7
  };

  if (c < 128)
    return c;
  else
    return table[c - 128];
}

const static struct {
  guint keycode;
  guint keyval;
  unsigned int modmask; /* So we can tell when a mod key is pressed/released */
} known_keys[] = {
  {  54, GDK_KEY_Meta_R,    NSCommandKeyMask },
  {  55, GDK_KEY_Meta_L,    NSCommandKeyMask },
  {  56, GDK_KEY_Shift_L,   NSShiftKeyMask },
  {  57, GDK_KEY_Caps_Lock, NSAlphaShiftKeyMask },
  {  58, GDK_KEY_Alt_L,     NSAlternateKeyMask },
  {  59, GDK_KEY_Control_L, NSControlKeyMask },
  {  60, GDK_KEY_Shift_R,   NSShiftKeyMask },
  {  61, GDK_KEY_Alt_R,     NSAlternateKeyMask },
  {  62, GDK_KEY_Control_R, NSControlKeyMask },
  { 122, GDK_KEY_F1, 0 },
  { 120, GDK_KEY_F2, 0 },
  {  99, GDK_KEY_F3, 0 },
  { 118, GDK_KEY_F4, 0 },
  {  96, GDK_KEY_F5, 0 },
  {  97, GDK_KEY_F6, 0 },
  {  98, GDK_KEY_F7, 0 },
  { 100, GDK_KEY_F8, 0 },
  { 101, GDK_KEY_F9, 0 },
  { 109, GDK_KEY_F10, 0 },
  { 103, GDK_KEY_F11, 0 },
  { 111, GDK_KEY_F12, 0 },
  { 105, GDK_KEY_F13, 0 },
  { 107, GDK_KEY_F14, 0 },
  { 113, GDK_KEY_F15, 0 },
  { 106, GDK_KEY_F16, 0 }
};

const static struct {
  guint keycode;
  guint normal_keyval, keypad_keyval;
} known_numeric_keys[] = {
  { 65, GDK_KEY_period, GDK_KEY_KP_Decimal },
  { 67, GDK_KEY_asterisk, GDK_KEY_KP_Multiply },
  { 69, GDK_KEY_plus, GDK_KEY_KP_Add },
  { 75, GDK_KEY_slash, GDK_KEY_KP_Divide },
  { 76, 0x01000003, GDK_KEY_KP_Enter },
  { 78, GDK_KEY_minus, GDK_KEY_KP_Subtract },
  { 81, GDK_KEY_equal, GDK_KEY_KP_Equal },
  { 82, GDK_KEY_0, GDK_KEY_KP_0 },
  { 83, GDK_KEY_1, GDK_KEY_KP_1 },
  { 84, GDK_KEY_2, GDK_KEY_KP_2 },
  { 85, GDK_KEY_3, GDK_KEY_KP_3 },
  { 86, GDK_KEY_4, GDK_KEY_KP_4 },
  { 87, GDK_KEY_5, GDK_KEY_KP_5 },
  { 88, GDK_KEY_6, GDK_KEY_KP_6 },
  { 89, GDK_KEY_7, GDK_KEY_KP_7 },
  { 91, GDK_KEY_8, GDK_KEY_KP_8 },
  { 92, GDK_KEY_9, GDK_KEY_KP_9 }
};

/* These values aren't covered by gdk_unicode_to_keyval */
const static struct {
  gunichar ucs_value;
  guint keyval;
} special_ucs_table [] = {
  { 0x0001, GDK_KEY_Home },
  { 0x0003, GDK_KEY_Return },
  { 0x0004, GDK_KEY_End },
  { 0x0008, GDK_KEY_BackSpace },
  { 0x0009, GDK_KEY_Tab },
  { 0x000b, GDK_KEY_Page_Up },
  { 0x000c, GDK_KEY_Page_Down },
  { 0x000d, GDK_KEY_Return },
  { 0x001b, GDK_KEY_Escape },
  { 0x001c, GDK_KEY_Left },
  { 0x001d, GDK_KEY_Right },
  { 0x001e, GDK_KEY_Up },
  { 0x001f, GDK_KEY_Down },
  { 0x007f, GDK_KEY_Delete },
  { 0xf060, GDK_KEY_dead_grave },
  { 0xf300, GDK_KEY_dead_grave },
  { 0xf0b4, GDK_KEY_dead_acute },
  { 0xf301, GDK_KEY_dead_acute },
  { 0xf385, GDK_KEY_dead_acute },
  { 0xf05e, GDK_KEY_dead_circumflex },
  { 0xf2c6, GDK_KEY_dead_circumflex },
  { 0xf302, GDK_KEY_dead_circumflex },
  { 0xf07e, GDK_KEY_dead_tilde },
  { 0xf303, GDK_KEY_dead_tilde },
  { 0xf342, GDK_KEY_dead_perispomeni },
  { 0xf0af, GDK_KEY_dead_macron },
  { 0xf304, GDK_KEY_dead_macron },
  { 0xf2d8, GDK_KEY_dead_breve },
  { 0xf306, GDK_KEY_dead_breve },
  { 0xf2d9, GDK_KEY_dead_abovedot },
  { 0xf307, GDK_KEY_dead_abovedot },
  { 0xf0a8, GDK_KEY_dead_diaeresis },
  { 0xf308, GDK_KEY_dead_diaeresis },
  { 0xf2da, GDK_KEY_dead_abovering },
  { 0xf30A, GDK_KEY_dead_abovering },
  { 0xf2dd, GDK_KEY_dead_doubleacute },
  { 0xf30B, GDK_KEY_dead_doubleacute },
  { 0xf2c7, GDK_KEY_dead_caron },
  { 0xf30C, GDK_KEY_dead_caron },
  { 0xf0be, GDK_KEY_dead_cedilla },
  { 0xf327, GDK_KEY_dead_cedilla },
  { 0xf2db, GDK_KEY_dead_ogonek },
  { 0xf328, GDK_KEY_dead_ogonek },
  { 0xfe5d, GDK_KEY_dead_iota },
  { 0xf323, GDK_KEY_dead_belowdot },
  { 0xf309, GDK_KEY_dead_hook },
  { 0xf31B, GDK_KEY_dead_horn },
  { 0xf02d, GDK_KEY_dead_stroke },
  { 0xf335, GDK_KEY_dead_stroke },
  { 0xf336, GDK_KEY_dead_stroke },
  { 0xf313, GDK_KEY_dead_abovecomma },
  /*  { 0xf313, GDK_KEY_dead_psili }, */
  { 0xf314, GDK_KEY_dead_abovereversedcomma },
  /*  { 0xf314, GDK_KEY_dead_dasia }, */
  { 0xf30F, GDK_KEY_dead_doublegrave },
  { 0xf325, GDK_KEY_dead_belowring },
  { 0xf2cd, GDK_KEY_dead_belowmacron },
  { 0xf331, GDK_KEY_dead_belowmacron },
  { 0xf32D, GDK_KEY_dead_belowcircumflex },
  { 0xf330, GDK_KEY_dead_belowtilde },
  { 0xf32E, GDK_KEY_dead_belowbreve },
  { 0xf324, GDK_KEY_dead_belowdiaeresis },
  { 0xf311, GDK_KEY_dead_invertedbreve },
  { 0xf02c, GDK_KEY_dead_belowcomma },
  { 0xf326, GDK_KEY_dead_belowcomma }
};
typedef struct _KBLayout KBLayout;
static UniChar get_keyvalue_uchr(KBLayout *layout, int key, UInt32 modifiers);
static UniChar get_keyvalue_kchr(KBLayout *layout, int key, UInt32 modifiers);
struct _KBLayout
{
    gpointer ref;
    UniChar (*get_keyvalue)(KBLayout*, int, UInt32);
    gpointer data;
};

#ifdef __LP64__

static KBLayout
get_keyboard_layout (void)
{
  KBLayout layout;
  CFDataRef layout_data_ref;

  layout.get_keyvalue = &get_keyvalue_uchr;
  layout.ref = (gpointer)TISCopyCurrentKeyboardLayoutInputSource ();

  layout_data_ref = (CFDataRef) TISGetInputSourceProperty
    ((TISInputSourceRef)layout.ref, kTISPropertyUnicodeKeyLayoutData);

  if (layout_data_ref)
    layout.data = CFDataGetBytePtr (layout_data_ref);

  if (layout.data == NULL)
    {
      g_error ("cannot get keyboard layout data");
    }
  return layout;
}

#else

static KBLayout
get_keyboard_layout (void)
{
  KBLayout layout;
  UInt32 data_type;
  SInt32 layout_kind;

  KLGetCurrentKeyboardLayout ((KeyboardLayoutRef*)&layout.ref);
  /* Get the layout kind */
  KLGetKeyboardLayoutProperty (layout.ref, kKLKind, (const void **)&layout_kind);
  if (layout_kind == kKLKCHRKind)
    {
      data_type = kKLKCHRData;
      layout.get_keyvalue = &get_keyvalue_kchr;
    }
  else
    {
      data_type = kKLuchrData;
      layout.get_keyvalue = &get_keyvalue_uchr;
    }
  /* Get chr data */
  KLGetKeyboardLayoutProperty (layout.ref, data_type, (const void **)&layout.data);
  return layout;
}
#endif

static UniChar
get_keyvalue_kchr(KBLayout *layout, int key, UInt32 modifiers)
{
#ifndef __LP64__
  UInt32 state = 0;
  UInt32 c;
  UInt16 key_code = modifiers | key;
  c = KeyTranslate (layout->data, key_code, &state);

  if (state != 0)
    {
      UInt32 state2 = 0;
      c = KeyTranslate (layout->data, key_code | 128, &state2);
    }

  if (c == 0 && c == 0x10)
    {
      return (UniChar)0;
    }
  return macroman2ucs (c);
#endif
}

static UniChar
get_keyvalue_uchr(KBLayout *layout, int key, UInt32 modifiers)
{
  UInt32 state = 0;
  OSStatus err;
  UniChar chars[4];
  UniCharCount nChars;

  err = UCKeyTranslate (layout->data, key,
                       kUCKeyActionDisplay,
                       (modifiers >> 8) & 0xFF,
                       LMGetKbdType(),
                       0,
                       &state, 4, &nChars, chars);

  if (err != noErr)
    return 0;
  /* A few <Shift><Option>keys return two characters, the first of
   * which is U+00a0, which isn't interesting; so we return the
   * second. More sophisticated handling is the job of a GtkIMContext.
   */
  /* If state isn't zero, it means that it's a dead key of some
     sort. Some of those are enumerated in the special_ucs_table with
     the high nibble set to f to push it into the private use
     range. Here we do the same.
  */
  if (state != 0)
    chars[nChars - 1] |= 0xf000;
  return chars[nChars -1];
}

static void
maybe_update_keymap (void)
{
  KBLayout new_layout = get_keyboard_layout();

  if (new_layout.ref == current_layout || new_layout.data == NULL)
    return;

  guint *p;
  int i;

  g_free (keyval_array);
  keyval_array = g_new0 (guint, NUM_KEYCODES * KEYVALS_PER_KEYCODE);

  for (i = 0; i < NUM_KEYCODES; i++)
    {
      int j;
      UInt32 modifiers[] = {0, shiftKey, optionKey, shiftKey | optionKey};

      p = keyval_array + i * KEYVALS_PER_KEYCODE;
	      
      for (j = 0; j < KEYVALS_PER_KEYCODE; j++)
	{
	  UniChar uc;
	  gboolean found = FALSE;
	  int k;

	  uc = new_layout.get_keyvalue(&new_layout, i, modifiers[j]);


	  for (k = 0; k < G_N_ELEMENTS (special_ucs_table); k++)
	    {
	      if (special_ucs_table[k].ucs_value == uc)
		{
		  p[j] = special_ucs_table[k].keyval;
		  found = TRUE;
		  break;
		}
	    }
		      
	  /* Special-case shift-tab since GTK+ expects
	   * GDK_KEY_ISO_Left_Tab for that.
	   */
	  if (found && p[j] == GDK_KEY_Tab && modifiers[j] == shiftKey)
	    p[j] = GDK_KEY_ISO_Left_Tab;

	  if (!found)
	    {
	      guint tmp;
                          
	      tmp = gdk_unicode_to_keyval (uc);
	      if (tmp != (uc | 0x01000000))
		p[j] = tmp;
	      else
		p[j] = 0;
	    }
	}

      if (p[3] == p[2])
	p[3] = 0;
      if (p[2] == p[1])
	p[2] = 0;
      if (p[1] == p[0])
	p[1] = 0;
      if (p[0] == p[2] &&
	  p[1] == p[3])
	p[2] = p[3] = 0;
    }
  for (i = 0; i < G_N_ELEMENTS (known_keys); i++)
    {
      p = keyval_array + known_keys[i].keycode * KEYVALS_PER_KEYCODE;

      if (p[0] == 0 && p[1] == 0 && 
	  p[2] == 0 && p[3] == 0)
	p[0] = known_keys[i].keyval;
    }

  for (i = 0; i < G_N_ELEMENTS (known_numeric_keys); i++)
    {
      p = keyval_array + known_numeric_keys[i].keycode * KEYVALS_PER_KEYCODE;

      if (p[0] == known_numeric_keys[i].normal_keyval)
	p[0] = known_numeric_keys[i].keypad_keyval;
    }
      
  if (current_layout)
    g_signal_emit_by_name (default_keymap, "keys_changed");

  current_layout = new_layout.ref;
}

GdkKeymap *
gdk_keymap_get_for_display (GdkDisplay *display)
{
  g_return_val_if_fail (display == gdk_display_get_default (), NULL);

  if (default_keymap == NULL)
    default_keymap = g_object_new (gdk_keymap_get_type (), NULL);

  return default_keymap;
}

PangoDirection
gdk_keymap_get_direction (GdkKeymap *keymap)
{
  return PANGO_DIRECTION_NEUTRAL;
}

gboolean
gdk_keymap_have_bidi_layouts (GdkKeymap *keymap)
{
  /* FIXME: Can we implement this? */
  return FALSE;
}

gboolean
gdk_keymap_get_caps_lock_state (GdkKeymap *keymap)
{
  /* FIXME: Implement this. */
  return FALSE;
}

gboolean
gdk_keymap_get_num_lock_state (GdkKeymap *keymap)
{
  /* FIXME: Implement this. */
  return FALSE;
}

gboolean
gdk_keymap_get_entries_for_keyval (GdkKeymap     *keymap,
                                   guint          keyval,
                                   GdkKeymapKey **keys,
                                   gint          *n_keys)
{
  GArray *keys_array;
  int i;

  g_return_val_if_fail (keymap == NULL || GDK_IS_KEYMAP (keymap), FALSE);
  g_return_val_if_fail (keys != NULL, FALSE);
  g_return_val_if_fail (n_keys != NULL, FALSE);
  g_return_val_if_fail (keyval != 0, FALSE);

  maybe_update_keymap ();

  *n_keys = 0;
  keys_array = g_array_new (FALSE, FALSE, sizeof (GdkKeymapKey));

  for (i = 0; i < NUM_KEYCODES * KEYVALS_PER_KEYCODE; i++)
    {
      GdkKeymapKey key;

      if (keyval_array[i] != keyval)
	continue;

      (*n_keys)++;

      key.keycode = i / KEYVALS_PER_KEYCODE;
      key.group = 0;
      key.level = i % KEYVALS_PER_KEYCODE;

      g_array_append_val (keys_array, key);
    }

  *keys = (GdkKeymapKey *)g_array_free (keys_array, FALSE);
  
  return *n_keys > 0;;
}

gboolean
gdk_keymap_get_entries_for_keycode (GdkKeymap     *keymap,
                                    guint          hardware_keycode,
                                    GdkKeymapKey **keys,
                                    guint        **keyvals,
                                    gint          *n_entries)
{
  GArray *keys_array, *keyvals_array;
  int i;
  guint *p;

  g_return_val_if_fail (keymap == NULL || GDK_IS_KEYMAP (keymap), FALSE);
  g_return_val_if_fail (n_entries != NULL, FALSE);

  maybe_update_keymap ();

  *n_entries = 0;

  if (hardware_keycode > NUM_KEYCODES)
    return FALSE;

  if (keys)
    keys_array = g_array_new (FALSE, FALSE, sizeof (GdkKeymapKey));
  else
    keys_array = NULL;

  if (keyvals)
    keyvals_array = g_array_new (FALSE, FALSE, sizeof (guint));
  else
    keyvals_array = NULL;

  p = keyval_array + hardware_keycode * KEYVALS_PER_KEYCODE;
  
  for (i = 0; i < KEYVALS_PER_KEYCODE; i++)
    {
      if (!p[i])
	continue;

      (*n_entries)++;
      
      if (keyvals_array)
	g_array_append_val (keyvals_array, p[i]);

      if (keys_array)
	{
	  GdkKeymapKey key;

	  key.keycode = hardware_keycode;
	  key.group = i / 2;
	  key.level = i % 2;

	  g_array_append_val (keys_array, key);
	}
    }
  
  if (keys)
    *keys = (GdkKeymapKey *)g_array_free (keys_array, FALSE);

  if (keyvals)
    *keyvals = (guint *)g_array_free (keyvals_array, FALSE);

  return *n_entries > 0;
}

guint
gdk_keymap_lookup_key (GdkKeymap          *keymap,
                       const GdkKeymapKey *key)
{
  g_return_val_if_fail (keymap == NULL || GDK_IS_KEYMAP (keymap), 0);
  g_return_val_if_fail (key != NULL, 0);
  g_return_val_if_fail (key->group < 4, 0);

  /* FIXME: Implement */

  return 0;
}

#define GET_KEYVAL(keycode, group, level) (keyval_array[(keycode * KEYVALS_PER_KEYCODE + group * 2 + level)])

static guint
translate_keysym (guint           hardware_keycode,
		  gint            group,
		  GdkModifierType state,
		  gint           *effective_group,
		  gint           *effective_level)
{
  gint level;
  guint tmp_keyval;

  level = (state & GDK_SHIFT_MASK) ? 1 : 0;

  if (!(GET_KEYVAL (hardware_keycode, group, 0) || GET_KEYVAL (hardware_keycode, group, 1)) &&
      (GET_KEYVAL (hardware_keycode, 0, 0) || GET_KEYVAL (hardware_keycode, 0, 1)))
    group = 0;

  if (!GET_KEYVAL (hardware_keycode, group, level) &&
      GET_KEYVAL (hardware_keycode, group, 0))
    level = 0;

  tmp_keyval = GET_KEYVAL (hardware_keycode, group, level);

  if (state & GDK_LOCK_MASK)
    {
      guint upper = gdk_keyval_to_upper (tmp_keyval);
      if (upper != tmp_keyval)
        tmp_keyval = upper;
    }

  return tmp_keyval;
}

gboolean
gdk_keymap_translate_keyboard_state (GdkKeymap       *keymap,
                                     guint            hardware_keycode,
                                     GdkModifierType  state,
                                     gint             group,
                                     guint           *keyval,
                                     gint            *effective_group,
                                     gint            *level,
                                     GdkModifierType *consumed_modifiers)
{
  guint tmp_keyval;
  GdkModifierType bit;
  guint tmp_modifiers = 0;

  g_return_val_if_fail (keymap == NULL || GDK_IS_KEYMAP (keymap), FALSE);
  g_return_val_if_fail (group >= 0 && group <= 1, FALSE);
  
  maybe_update_keymap ();

  if (keyval)
    *keyval = 0;
  if (effective_group)
    *effective_group = 0;
  if (level)
    *level = 0;
  if (consumed_modifiers)
    *consumed_modifiers = 0;

  if (hardware_keycode < 0 || hardware_keycode >= NUM_KEYCODES)
    return FALSE;
  
  /* Check if shift or capslock modify the keyval */
  for (bit = GDK_SHIFT_MASK; bit < GDK_CONTROL_MASK; bit <<= 1)
    {
      if (translate_keysym (hardware_keycode, group, state & ~bit, NULL, NULL) !=
	  translate_keysym (hardware_keycode, group, state | bit, NULL, NULL))
	tmp_modifiers |= bit;
    }

  tmp_keyval = translate_keysym (hardware_keycode, group, state, level, effective_group);

  if (consumed_modifiers)
    *consumed_modifiers = tmp_modifiers;

  if (keyval)
    *keyval = tmp_keyval; 

  return TRUE;
}

void
gdk_keymap_add_virtual_modifiers (GdkKeymap       *keymap,
                                  GdkModifierType *state)
{
  if (*state & GDK_MOD1_MASK)
    *state |= GDK_META_MASK;
  if (*state & GDK_MOD5_MASK)
    *state |= GDK_SUPER_MASK;

}

gboolean
gdk_keymap_map_virtual_modifiers (GdkKeymap       *keymap,
                                  GdkModifierType *state)
{
  if (*state & GDK_META_MASK)
    *state |= GDK_MOD1_MASK;
  if (*state & GDK_SUPER_MASK)
    *state |= GDK_MOD5_MASK;

  return TRUE;
}

/* What sort of key event is this? Returns one of
 * GDK_KEY_PRESS, GDK_KEY_RELEASE, GDK_NOTHING (should be ignored)
 */
GdkEventType
_gdk_quartz_keys_event_type (NSEvent *event)
{
  unsigned short keycode;
  unsigned int flags;
  int i;
  
  switch ([event type])
    {
    case NSKeyDown:
      return GDK_KEY_PRESS;
    case NSKeyUp:
      return GDK_KEY_RELEASE;
    case NSFlagsChanged:
      break;
    default:
      g_assert_not_reached ();
    }
  
  /* For flags-changed events, we have to find the special key that caused the
   * event, and see if it's in the modifier mask. */
  keycode = [event keyCode];
  flags = [event modifierFlags];
  
  for (i = 0; i < G_N_ELEMENTS (known_keys); i++)
    {
      if (known_keys[i].keycode == keycode)
	{
	  if (flags & known_keys[i].modmask)
	    return GDK_KEY_PRESS;
	  else
	    return GDK_KEY_RELEASE;
	}
    }
  
  /* Some keypresses (eg: Expose' activations) seem to trigger flags-changed
   * events for no good reason. Ignore them! */
  return GDK_NOTHING;
}

gboolean
_gdk_quartz_keys_is_modifier (guint keycode)
{
  gint i;
  
  for (i = 0; i < G_N_ELEMENTS (known_keys); i++)
    {
      if (known_keys[i].modmask == 0)
	break;

      if (known_keys[i].keycode == keycode)
	return TRUE;
    }

  return FALSE;
}
