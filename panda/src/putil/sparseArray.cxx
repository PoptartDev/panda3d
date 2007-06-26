// Filename: sparseArray.cxx
// Created by:  drose (14Feb07)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#include "sparseArray.h"
#include "bitArray.h"

TypeHandle SparseArray::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: SparseArray::Constructor (from BitArray)
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
SparseArray::
SparseArray(const BitArray &from) {
  bool empty_bit = from.get_highest_bits();
  _inverse = empty_bit;

  int begin = 0;
  bool current_state = from.get_bit(0);
  int i = 0;

  // By including get_num_bits()--one more than the last bit--in this
  // traversal, we guarantee that we will end on the empty_bit state
  // (because the last bit we visit will be one of the highest_bits).
  while (i <= from.get_num_bits()) {
    if (from.get_bit(i) != current_state) {
      // End of a run.
      if (current_state != empty_bit) {
        Subrange range(begin, i);
        _subranges.push_back(range);
      }
      begin = i;
      current_state = !current_state;
    }
    ++i;
  }

  nassertv(current_state == empty_bit);
}

////////////////////////////////////////////////////////////////////
//     Function: SparseArray::get_num_on_bits
//       Access: Published
//  Description: Returns the number of bits that are set to 1 in the
//               array.  Returns -1 if there are an infinite number of
//               1 bits.
////////////////////////////////////////////////////////////////////
int SparseArray::
get_num_on_bits() const {
  if (_inverse) {
    return -1;
  }

  int result = 0;
  Subranges::const_iterator si;
  for (si = _subranges.begin(); si != _subranges.end(); ++si) {
    result += (*si)._end - (*si)._begin;
  }

  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: SparseArray::get_num_off_bits
//       Access: Published
//  Description: Returns the number of bits that are set to 0 in the
//               array.  Returns -1 if there are an infinite number of
//               0 bits.
////////////////////////////////////////////////////////////////////
int SparseArray::
get_num_off_bits() const {
  if (!_inverse) {
    return -1;
  }

  int result = 0;
  Subranges::const_iterator si;
  for (si = _subranges.begin(); si != _subranges.end(); ++si) {
    result += (*si)._end - (*si)._begin;
  }

  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: SparseArray::get_next_higher_different_bit
//       Access: Published
//  Description: Returns the index of the next bit in the array, above
//               low_bit, whose value is different that the value of
//               low_bit.  Returns low_bit again if all bits higher
//               than low_bit have the same value.
//
//               This can be used to quickly iterate through all of
//               the bits in the array.
////////////////////////////////////////////////////////////////////
int SparseArray::
get_next_higher_different_bit(int low_bit) const {
  Subrange range(low_bit, low_bit + 1);
  Subranges::const_iterator si = _subranges.lower_bound(range);
  if (si == _subranges.end()) {
    // That was the end of the array.
    return low_bit;
  }

  if (low_bit >= (*si)._begin) {
    return (*si)._end;
  }

  int next = (*si)._begin;

  if (si != _subranges.begin()) {
    --si;
    if (low_bit < (*si)._end) {
      return (*si)._end;
    }
  }

  return next;
}

////////////////////////////////////////////////////////////////////
//     Function: SparseArray::has_bits_in_common
//       Access: Published
//  Description: Returns true if this SparseArray has any "one" bits in
//               common with the other one, false otherwise.
//
//               This is equivalent to (array & other) != 0, but may
//               be faster.
////////////////////////////////////////////////////////////////////
bool SparseArray::
has_bits_in_common(const SparseArray &other) const {
  if (_inverse && other._inverse) {
    // Yup, in fact we have an infinite number of bits in common.
    return true;
  }

  if (_inverse != other._inverse) {
    // We'll handle this tricky case the lazy way.
    return !(*this & other).is_zero();
  }

  // Actually, we'll handle this easy case the lazy way too.  Maybe
  // later we'll do this smarter.
  return !(*this & other).is_zero();
}

////////////////////////////////////////////////////////////////////
//     Function: SparseArray::output
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void SparseArray::
output(ostream &out) const {
  out << "[ ";
  if (_inverse) {
    out << "all except: ";
  }
  Subranges::const_iterator si;
  for (si = _subranges.begin(); si != _subranges.end(); ++si) {
    if ((*si)._end == (*si)._begin + 1) {
      // A single element.
      out << (*si)._begin << ", ";
    } else {
      // A range of elements.
      out << (*si)._begin << "-" << ((*si)._end - 1) << ", ";
    }
  }
  out << "]";
}

////////////////////////////////////////////////////////////////////
//     Function: SparseArray::compare_to
//       Access: Published
//  Description: Returns a number less than zero if this SparseArray
//               sorts before the indicated other SparseArray, greater
//               than zero if it sorts after, or 0 if they are
//               equivalent.  This is based on the same ordering
//               defined by operator <.
////////////////////////////////////////////////////////////////////
int SparseArray::
compare_to(const SparseArray &other) const {
  if (_inverse != other._inverse) {
    return _inverse ? 1 : -1;
  }

  Subranges::const_reverse_iterator ai = _subranges.rbegin();
  Subranges::const_reverse_iterator bi = other._subranges.rbegin();

  while (ai != _subranges.rend() && bi != other._subranges.rend()) {
    if ((*ai)._end < (*bi)._end) {
      // B is higher.
      return -1;
    } else if ((*bi)._end < (*ai)._end) {
      // A is higher.
      return 1;
    } else if ((*ai)._begin < (*bi)._begin) {
      // A is higher.
      return 1;
    } else if ((*bi)._begin < (*ai)._begin) {
      // B is higher.
      return -1;
    }

    --ai;
    --bi;
  }

  if (ai != _subranges.rend()) {
    // A is higher.
    return 1;
  }
  if (bi != other._subranges.rend()) {
    // B is higher.
    return -1;
  }

  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: SparseArray::operator &=
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void SparseArray::
operator &= (const SparseArray &other) {
  // We do this the slow and stupid way.  This could be done much
  // better with a little effort, but I'm not at all sure it's worth
  // the effort.  If you need fast boolean operations, you should
  // probably be using a BitArray.

  if (_inverse && other._inverse) {
    do_union(other);

  } else if (!_inverse && !other._inverse) {
    do_intersection(other);

  } else if (_inverse && !other._inverse) {
    // a & b == b & a
    (*this) = other & (*this);

  } else if (!_inverse && other._inverse) {
    do_intersection_neg(other);

  } else {
    // TODO.
    nassertv(false);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: SparseArray::operator |=
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void SparseArray::
operator |= (const SparseArray &other) {
  // We do this the slow and stupid way.  This could be done much
  // better with a little effort, but I'm not at all sure it's worth
  // the effort.  If you need fast boolean operations, you should
  // probably be using a BitArray.

  if (_inverse && other._inverse) {
    do_intersection(other);

  } else if (!_inverse && !other._inverse) {
    do_union(other);

  } else if (_inverse && !other._inverse) {
    do_intersection_neg(other);

  } else if (!_inverse && other._inverse) {
    // a | b == b | a
    (*this) = other | (*this);

  } else {
    nassertv(false);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: SparseArray::operator ^=
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void SparseArray::
operator ^= (const SparseArray &other) {
  // We do this the slow and stupid way.  This could be done much
  // better with a little effort, but I'm not at all sure it's worth
  // the effort.  If you need fast boolean operations, you should
  // probably be using a BitArray.

  (*this) = ((*this) | other) & ~((*this) & other);
}

////////////////////////////////////////////////////////////////////
//     Function: SparseArray::do_add_range
//       Access: Private
//  Description: Adds the consecutive range of integers beginning at
//               begin, but not including end, to the array.  If this
//               range overlaps with another range already in the
//               array, the result is the union.
////////////////////////////////////////////////////////////////////
void SparseArray::
do_add_range(int begin, int end) {
  if (begin >= end) {
    // Empty range.
    return;
  }

  Subrange range(begin, end);
  Subranges::iterator si = _subranges.lower_bound(range);
  if (si == _subranges.end()) {
    if (!_subranges.empty()) {
      si = _subranges.begin() + _subranges.size() - 1;
      if ((*si)._end >= begin) {
        // The new range expands the last element of the array to the right.
        (*si)._end = end;
        // It might also expand it to the left; fall through.
      } else {
        // The new range is completely after the last element of the array.
        _subranges.push_back(range);
        return;
      }

    } else {
      // The new range is completely after the last element of the array.
      _subranges.push_back(range);
      return;
    }
  }

  nassertv((*si)._end >= end);

  if ((*si)._begin > end) {
    if (si != _subranges.begin()) {
      Subranges::iterator si2 = si;
      --si2;
      if ((*si2)._end >= begin) {
        // The new range expands an element within the array to the
        // right (but does not intersect the next element).
        (*si2)._end = end;
        // It might also expand it to the left; fall through.
        si = si2;
      } else {
        // The new range does not intersect any elements in the array.
        _subranges.insert_unverified(si, range);
        return;
      }
    } else {
      // The new range does not intersect any elements in the array.
      _subranges.insert_unverified(si, range);
      return;
    }
  }

  // Check if the new range overlaps with any elements to the left.
  while (si != _subranges.begin()) {
    Subranges::iterator si2 = si;
    --si2;
    if ((*si2)._end >= begin) {
      // The new range straddles two elements, so they get combined.
      (*si2)._end = (*si)._end;
      _subranges.erase(si);
    } else {
      // Stop here.
      break;
    }
    si = si2;
  }

  if ((*si)._begin > begin) {
    // The new range expands an element to the left.
    (*si)._begin = begin;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: SparseArray::do_remove_range
//       Access: Private
//  Description: Removes the consecutive range of integers beginning
//               at begin, but not including end, from the array.
////////////////////////////////////////////////////////////////////
void SparseArray::
do_remove_range(int begin, int end) {
  if (begin >= end) {
    // Empty range.
    return;
  }

  Subrange range(begin, end);
  Subranges::iterator si = _subranges.lower_bound(range);
  if (si == _subranges.end()) {
    if (!_subranges.empty()) {
      si = _subranges.begin() + _subranges.size() - 1;
      if ((*si)._end >= begin) {
        // The new range shortens the last element of the array on the right.
        end = min(end, (*si)._begin);
        (*si)._end = end;
        // It might also shorten it on the left; fall through.
      } else {
        // The new range is completely after the last element of the array.
        return;
      }

    } else {
      // The new range is completely after the last element of the array.
      return;
    }
  }

  nassertv((*si)._end >= end);

  if ((*si)._begin > end) {
    if (si != _subranges.begin()) {
      Subranges::iterator si2 = si;
      --si2;
      if ((*si2)._end >= begin) {
        // The new range shortens an element within the array on the
        // right (but does not intersect the next element).
        end = min(end, (*si2)._begin);
        (*si2)._end = end;
        // It might also shorten it on the left; fall through.
        si = si2;
      } else {
        // The new range does not intersect any elements in the array.
        return;
      }
    } else {
      // The new range does not intersect any elements in the array.
      return;
    }
  }


  if (end < (*si)._end) {
    // We must split an element into two.
    Subrange left_range((*si)._begin, begin);
    (*si)._begin = end;
    si = _subranges.insert_unverified(si, left_range);
  }

  // Check if the new range removes any elements to the left.
  while (begin <= (*si)._begin) {
    if (si == _subranges.begin()) {
      _subranges.erase(si);
      return;
    }
    Subranges::iterator si2 = si;
    --si2;
    _subranges.erase(si);
    si = si2;
  }

  (*si)._end = min((*si)._end, begin);
}

////////////////////////////////////////////////////////////////////
//     Function: SparseArray::do_has_any
//       Access: Private
//  Description: Returns true if any of the consecutive range of
//               integers beginning at begin, but not including end,
//               appear in the array.  Note that this will return
//               false for an empty range.
////////////////////////////////////////////////////////////////////
bool SparseArray::
do_has_any(int begin, int end) const {
  if (begin >= end) {
    // Empty range.
    return false;
  }

  Subrange range(begin, end);
  Subranges::const_iterator si = _subranges.lower_bound(range);
  if (si != _subranges.end() && end > (*si)._begin) {
    return true;
  }
  if (si != _subranges.begin()) {
    --si;
    if (begin < (*si)._end) {
      return true;
    }
  }

  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: SparseArray::do_has_all
//       Access: Private
//  Description: Returns true if all of the consecutive range of
//               integers beginning at begin, but not including end,
//               appear in the array.  Note that this will return
//               true for an empty range.
////////////////////////////////////////////////////////////////////
bool SparseArray::
do_has_all(int begin, int end) const {
  if (begin >= end) {
    // Empty range.
    return true;
  }

  Subrange range(begin, end);
  Subranges::const_iterator si = _subranges.lower_bound(range);
  if (si != _subranges.end() && begin >= (*si)._begin) {
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: SparseArray::do_intersection
//       Access: Private
//  Description: Removes from this array all of the elements that do
//               not appear in the other one.
////////////////////////////////////////////////////////////////////
void SparseArray::
do_intersection(const SparseArray &other) {
  if (_subranges.empty()) {
    return;
  }
  if (other._subranges.empty()) {
    _subranges.clear();
    return;
  }

  int my_begin = (*_subranges.begin())._begin;
  int other_begin = (*other._subranges.begin())._begin;
  do_remove_range(my_begin, other_begin);

  for (size_t i = 0; i < other._subranges.size() - 1; ++i) {
    do_remove_range(other._subranges[i]._end, other._subranges[i + 1]._begin);
  }

  int my_end = (*(_subranges.begin() + _subranges.size() - 1))._end;
  int other_end = (*(other._subranges.begin() + other._subranges.size() - 1))._end;
  do_remove_range(other_end, my_end);
}

////////////////////////////////////////////////////////////////////
//     Function: SparseArray::do_union
//       Access: Private
//  Description: Adds to this array all of the elements that also
//               appear in the other one.
////////////////////////////////////////////////////////////////////
void SparseArray::
do_union(const SparseArray &other) {
  Subranges::const_iterator si;
  for (si = other._subranges.begin(); si != other._subranges.end(); ++si) {
    do_add_range((*si)._begin, (*si)._end);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: SparseArray::do_intersection_neg
//       Access: Private
//  Description: Removes from this array all of the elements that also
//               appear in the other one.
////////////////////////////////////////////////////////////////////
void SparseArray::
do_intersection_neg(const SparseArray &other) {
  Subranges::const_iterator si;
  for (si = other._subranges.begin(); si != other._subranges.end(); ++si) {
    do_remove_range((*si)._begin, (*si)._end);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: SparseArray::do_shift
//       Access: Private
//  Description: Shifts all the elements in the array by the indicated
//               amount.
////////////////////////////////////////////////////////////////////
void SparseArray::
do_shift(int offset) {
  if (offset != 0) {
    Subranges::iterator si;
    for (si = _subranges.begin(); si != _subranges.end(); ++si) {
      (*si)._begin += offset;
      (*si)._end += offset;
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: SparseArray::write_datagram
//       Access: Public
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void SparseArray::
write_datagram(BamWriter *manager, Datagram &dg) const {
  dg.add_uint32(_subranges.size());
  Subranges::const_iterator si;
  for (si = _subranges.begin(); si != _subranges.end(); ++si) {
    dg.add_int32((*si)._begin);
    dg.add_int32((*si)._end);
  }
  dg.add_bool(_inverse);
}

////////////////////////////////////////////////////////////////////
//     Function: SparseArray::read_datagram
//       Access: Public
//  Description: Reads the object that was previously written to a Bam
//               file.
////////////////////////////////////////////////////////////////////
void SparseArray::
read_datagram(DatagramIterator &scan, BamReader *manager) {
  size_t num_subranges = scan.get_uint32();
  _subranges.reserve(num_subranges);
  for (size_t i = 0; i < num_subranges; ++i) {
    int begin = scan.get_int32();
    int end = scan.get_int32();
    _subranges.push_back(Subrange(begin, end));
  }
  _inverse = scan.get_bool();
}
