/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pnmTextGlyph.I
 * @author drose
 * @date 2003-09-07
 */

/**
 * Returns the number of pixels by which the pen should be advanced after
 * rendering this glyph.
 */
INLINE int PNMTextGlyph::
get_advance() const {
  return _int_advance;
}

/**
 * Returns the x coordinate of the leftmost pixel in the glyph.
 */
INLINE int PNMTextGlyph::
get_left() const {
  return _left;
}

/**
 * Returns the x coordinate of the rightmost pixel in the glyph.
 */
INLINE int PNMTextGlyph::
get_right() const {
  return _left + _image.get_x_size();
}

/**
 * Returns the y coordinate of the bottommost pixel in the glyph.
 */
INLINE int PNMTextGlyph::
get_bottom() const {
  return _top + _image.get_y_size();
}

/**
 * Returns the y coordinate of the topmost pixel in the glyph.
 */
INLINE int PNMTextGlyph::
get_top() const {
  return _top;
}

/**
 * Returns the height of the glyph in pixels.
 */
INLINE int PNMTextGlyph::
get_height() const {
  return _image.get_y_size();
}

/**
 * Returns the width of the glyph in pixels.
 */
INLINE int PNMTextGlyph::
get_width() const {
  return _image.get_x_size();
}

/**
 * Returns the value of the indicated pixel of the glyph.  The result is in
 * the range [0, 1], where 0 indicates the pixel is not part of the glyph, and
 * 1 indicates it is.  Intermediate values are used to represent antialiasing.
 */
INLINE double PNMTextGlyph::
get_value(int x, int y) const {
  nassertr(x >= 0 && x < get_width() &&
           y >= 0 && y < get_height(), 0.0);
  // By convention, the "value" attribute is stored in the blue component.
  return _image.get_blue(x, y);
}

/**
 * Returns true if the indicated pixel represents a pixel in the interior of a
 * hollow font, false otherwise.
 */
INLINE bool PNMTextGlyph::
get_interior_flag(int x, int y) const {
  nassertr(x >= 0 && x < get_width() &&
           y >= 0 && y < get_height(), false);
  // By convention, the "interior_value" attribute is stored in the red
  // component.
  return _image.get_red_val(x, y) != 0;
}
