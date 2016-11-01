/*
 * (C) Copyright 1996-2016 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#ifndef atlas_Array_h
#define atlas_Array_h

#include <vector>
#include <iosfwd>
#include <iterator>
#include "eckit/memory/Owned.h"
#include "eckit/exception/Exceptions.h"
#include "atlas/array/ArrayUtil.h"
#include "atlas/array/DataType.h"
#include "atlas/array/ArrayView.h"
#include "atlas/internals/atlas_config.h"
#ifdef ATLAS_HAVE_GRIDTOOLS_STORAGE
#include "storage-facility.hpp"
#endif

//------------------------------------------------------------------------------

namespace atlas {
namespace array {

class Array : public eckit::Owned {
public:
  static Array* create( array::DataType, const ArrayShape& );
  static Array* create( array::DataType );
  static Array* create( const Array& );
  template <typename T> static Array* create(const ArrayShape& s);
  template <typename T> static Array* create();
  template <typename T> static Array* create(size_t size);
  template <typename T> static Array* create(size_t size1, size_t size2);
  template <typename T> static Array* create(size_t size1, size_t size2, size_t size3);
  template <typename T> static Array* create(size_t size1, size_t size2, size_t size3, size_t size4);

  template <typename T> static Array* wrap(T data[], const ArraySpec&);
  template <typename T> static Array* wrap(T data[], const ArrayShape&);

public:

  Array(){}
  Array(const ArraySpec& s) : spec_(s) {}

  virtual array::DataType datatype() const = 0;
  virtual double bytes() const = 0;
  virtual void dump(std::ostream& os) const = 0;

  void resize(const ArrayShape&);

  void resize(size_t size1);

  void resize(size_t size1, size_t size2);

  void resize(size_t size1, size_t size2, size_t size3);

  void resize(size_t size1, size_t size2, size_t size3, size_t size4);

  void insert(size_t idx1, size_t size1);

  size_t size() const { return spec_.size(); }

  size_t rank() const { return spec_.rank(); }

  size_t stride(size_t i) const { return spec_.strides()[i]; }

  size_t shape(size_t i) const { return spec_.shape()[i]; }

  const ArrayStrides& strides() const { return spec_.strides(); }

  const ArrayShape& shape() const { return spec_.shape(); }

  const std::vector<int>& shapef() const { return spec_.shapef(); }

  const std::vector<int>& stridesf() const { return spec_.stridesf(); }

  bool contiguous() const { return spec_.contiguous(); }

  void operator=( const Array &array ) { return assign(array); }
  virtual void assign( const Array& )=0;

private: // methods

  virtual void resize_data( size_t size )=0;
  virtual void insert_data(size_t idx1, size_t size1)=0;

private:

  ArraySpec spec_;
};

template <typename T> Array* Array::create(const ArrayShape& s)
{ return create(array::DataType::create<T>(),s); }

template <typename T> Array* Array::create()
{ return create(array::DataType::create<T>()); }

template <typename T> Array* Array::create(size_t size)
{ return create(array::DataType::create<T>(),make_shape(size)); }

template <typename T> Array* Array::create(size_t size1, size_t size2)
{ return create(array::DataType::create<T>(),make_shape(size1,size2)); }

template <typename T> Array* Array::create(size_t size1, size_t size2, size_t size3)
{ return create(array::DataType::create<T>(),make_shape(size1,size2,size3)); }

template <typename T> Array* Array::create(size_t size1, size_t size2, size_t size3, size_t size4)
{ return create(array::DataType::create<T>(),make_shape(size1,size2,size3,size4)); }

//------------------------------------------------------------------------------

template< typename DATA_TYPE >
class ArrayT : public Array  {
public:

  typedef typename remove_const<DATA_TYPE>::type  value_type;
  typedef typename add_const<DATA_TYPE>::type     const_value_type;

public:

  ArrayT(): owned_(true) {}

  ArrayT(const ArrayShape& shape): owned_(true)                                { resize(shape); }

  ArrayT(size_t size): owned_(true)                                            { resize( make_shape(size) ); }

  ArrayT(size_t size1, size_t size2): owned_(true)                             { resize( make_shape(size1,size2) ); }

  ArrayT(size_t size1, size_t size2, size_t size3): owned_(true)               { resize( make_shape(size1,size2,size3) ); }

  ArrayT(size_t size1, size_t size2, size_t size3, size_t size4): owned_(true) { resize( make_shape(size1,size2,size3,size4) ); }

  ArrayT(DATA_TYPE data[], const ArraySpec& spec):
    Array(spec),
    owned_(false)
  { wrap(data); }

  ArrayT(DATA_TYPE data[], const ArrayShape& shape):
    Array(ArraySpec(shape)),
    owned_(false)
  { wrap(data); }

public:

  virtual array::DataType datatype() const { return array::DataType::create<DATA_TYPE>(); }
  virtual double bytes() const { return sizeof(DATA_TYPE)*size(); }
  virtual void dump(std::ostream& os) const;

  const DATA_TYPE& operator[](size_t i) const { return *(data()+i); }
        DATA_TYPE& operator[](size_t i)       { return *(data()+i); }

  const DATA_TYPE* data() const { return data_; }
        DATA_TYPE* data()       { return data_; }

  void operator=(const DATA_TYPE& scalar) { for(size_t n=0; n<size(); ++n) data_[n]=scalar; }

  virtual void assign( const Array& );

  template< typename RandomAccessIterator >
  void assign( RandomAccessIterator begin, RandomAccessIterator end );

  virtual void insert_data(size_t idx1, size_t size1);

private:

  virtual void resize_data( size_t size );
  void wrap(DATA_TYPE data[]);

private:
  bool owned_;
  std::vector<DATA_TYPE> owned_data_;
  DATA_TYPE* data_;
};


template< typename DATA_TYPE>
void ArrayT<DATA_TYPE>::resize_data( size_t size )
{
  if( !owned_ ) throw eckit::SeriousBug("Cannot resize data that is not owned");
  owned_data_.resize( size );
  data_ = owned_data_.data();
}

template< typename DATA_TYPE>
void ArrayT<DATA_TYPE>::insert_data(size_t pos, size_t size)
{
  if( !owned_ ) throw eckit::SeriousBug("Cannot resize data that is not owned");
  owned_data_.insert(owned_data_.begin()+pos,size,0);
  data_ = owned_data_.data();
}


template <typename DATA_TYPE>
void ArrayT<DATA_TYPE>::wrap(DATA_TYPE data[])
{
  data_ = data;
}

template< typename DATA_TYPE>
template< typename RandomAccessIterator >
void ArrayT<DATA_TYPE>::assign( RandomAccessIterator begin, RandomAccessIterator end )
{
  if( not contiguous() ) NOTIMP;
  if( std::distance(begin,end) != size() ) {
    throw eckit::SeriousBug("Size doesn't match");
  }
  RandomAccessIterator it = begin;
  for( size_t j=0; j<size(); ++j, ++it ) {
    data_[j] = *it;
  }
}

template< typename DATA_TYPE>
void ArrayT<DATA_TYPE>::assign( const Array& other )
{
  if( not contiguous() or not other.contiguous()) NOTIMP;
  resize( other.shape() );
  ASSERT( datatype().kind() == other.datatype().kind() );
  const ArrayT<DATA_TYPE>& other_array = *dynamic_cast< const ArrayT<DATA_TYPE>* >(&other);
  
  const DATA_TYPE* other_data = other_array.data();
  for( size_t j=0; j<size(); ++j )
    data_[j] = other_data[j];
}

//------------------------------------------------------------------------------

} // namespace array
} // namespace atlas

#endif
