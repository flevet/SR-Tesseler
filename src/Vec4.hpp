/*
* Software:  SR-Tesseler (Multiscale segmentation of localization-based super-resolution microscopy data with polygons)
*
* File:      Vec4.hpp
*
* Copyright: Florian Levet (2010-2019)
*
* License:   GPL v3
*
* Homepage:  http://www.iins.u-bordeaux.fr/team-sibarita-SR-Tesseler
*
*
* SR-Tesseler is a free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 3 of the License, or
* (at your option) any later version, provided that this entire notice
* is included in all copies of any software which is or includes a copy
* or modification of this software and in all copies of the supporting
* documentation for such software.
*
* The algorithms that underlie SR-Tesseler have required considerable
* development. They are described in the original SR-Tesseler paper,
* doi:10.1038/nmeth.3579. If you use SR-Tesseler as part of work towards a
* scientific publication, please include a citation to the original paper.
*
* SR-Tesseler is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*/

#ifndef Vec4_h__
#define Vec4_h__

#include < iostream >

template< class T >
      class Vec4
      {

         public:

         /*----- methods -----*/

         inline static Vec4 zero();
   
         /*----- methods -----*/

         template< class S > Vec4( const Vec4<S>& vec )
         {
            _e[0] = (T)vec(0);
            _e[1] = (T)vec(1);
			_e[2] = (T)vec(2);
			_e[3] = (T)vec(3);
         }

         Vec4() { _e[0]=_e[1]=_e[2]=_e[3]=0; }
         Vec4( const Vec4<T>& vec );
         Vec4( const T& e0, const T& e1, const T& e2, const T& e3 );

         ~Vec4() {}

         T* ptr();
         const T* ptr() const;

         T* getArray();
		 T* getValues();
         const T* getArray() const;
		 const T* getValues() const;

         T length() const;
		 T lengthSquare() const;
         T sqrLength() const;
         T dot( const Vec4<T>& vec ) const;
		 Vec4<T> cross( const Vec4<T>& vec ) const;

         Vec4  normal() const;
         Vec4& normalEq();
		 Vec4& normalize();
         Vec4& normalEq( const T len );
         Vec4& negateEq();
         Vec4& clampToMaxEq( const T& max );
           
         Vec4 operator+( const Vec4<T>& rhs ) const;
         Vec4 operator-( const Vec4<T>& rhs ) const;
         Vec4 operator-() const;
         Vec4 operator*( const T& rhs ) const;
         Vec4 operator*( const Vec4<T>& rhs ) const;
         Vec4 operator/( const T& rhs ) const;
         Vec4 operator/( const Vec4<T>& rhs ) const;

         Vec4& operator+=( const Vec4<T>& rhs );
         Vec4& operator-=( const Vec4<T>& rhs );
         Vec4& operator*=( const T& rhs );
         Vec4& operator*=( const Vec4<T>& rhs );
         Vec4& operator/=( const T& rhs );
         Vec4& operator/=( const Vec4<T>& rhs );
         Vec4& operator=( const Vec4<T>& rsh );

         bool operator==( const Vec4<T>& rhs ) const;
         bool operator!=( const Vec4<T>& rhs ) const;

         T& operator()( int idx );
         const T& operator()( int idx ) const;

         T& operator[]( int idx );
         const T& operator[]( int idx ) const;

         void set( T const x, T const y, T const z, T const w );
		 void setX(const T & x);
		 void setY(const T & y);
		 void setZ(const T & z);
		 void setW(const T & w);
           
           
         T& x();
         T& y();
		 T& z();
		 T& w();

         const T& x() const;
         const T& y() const;
		 const T& z() const;
		 const T& w() const;
   
         private:

         /*----- data members -----*/

         T _e[4];
      };

//------------------------------------------------------------------------------
//!
      template< class T >
      inline Vec4<T> Vec4<T>::zero() 
      {
         return Vec4( 0, 0, 0, 0 );
      }

//------------------------------------------------------------------------------
//!
      template< class T >
      inline Vec4<T>::Vec4( const Vec4<T>& vec )
      {
         _e[0] = vec._e[0];
         _e[1] = vec._e[1];
		 _e[2] = vec._e[2];
		 _e[3] = vec._e[3];
      }

//------------------------------------------------------------------------------
//!
      template< class T >
      inline Vec4<T>::Vec4( const T& e0, const T& e1, const T& e2, const T& e3 )
      {
         _e[0] = e0;
         _e[1] = e1;
		 _e[2] = e2;
		 _e[3] = e3;
      }

//------------------------------------------------------------------------------
//!
      template< class T >
      inline T* Vec4<T>::ptr()
      {
         return _e;
      }

//------------------------------------------------------------------------------
//!
      template< class T >
      inline const T* Vec4<T>::ptr() const
      {
         return _e;
      }

//------------------------------------------------------------------------------
//!
      template< class T >
      inline T* Vec4<T>::getArray()
      {
         return _e;
      }

//------------------------------------------------------------------------------
//!
      template< class T >
      inline T* Vec4<T>::getValues()
      {
         return _e;
      }
//------------------------------------------------------------------------------
//!
      template< class T >
      inline const T* Vec4<T>::getArray() const
      {
         return _e;
      }

//------------------------------------------------------------------------------
//!
      template< class T >
      inline const T* Vec4<T>::getValues() const
      {
         return _e;
      }
//------------------------------------------------------------------------------
//!
      template< class T >
      inline T Vec4<T>::length() const
      {
         return (T)sqrt( _e[0]*_e[0] + _e[1]*_e[1] + _e[2]*_e[2] + _e[3]*_e[3]);
      }

//------------------------------------------------------------------------------
//!
      template< class T >
      inline T Vec4<T>::lengthSquare() const
      {
         return _e[0]*_e[0] + _e[1]*_e[1] + _e[2]*_e[2] + _e[3]*_e[3];
      }

//------------------------------------------------------------------------------
//!
      template< class T >
      inline T Vec4<T>::sqrLength() const
      {
         return _e[0]*_e[0] + _e[1]*_e[1] + _e[2]*_e[2] + _e[3]*_e[3];
      }

//------------------------------------------------------------------------------
//!
      template< class T >
      inline T Vec4<T>::dot( const Vec4<T>& vec ) const
      {
         return _e[0]*vec._e[0] + _e[1]*vec._e[1] + _e[2]*vec._e[2] + _e[3]*vec._e[3];
      }

//------------------------------------------------------------------------------
//!

	  template< class T >
      inline Vec4<T> Vec4<T>::cross( const Vec4<T>& rhs ) const
      {
		  Vec4<T> res;
		  res[0] = _e[1]*rhs[2] - _e[2]*rhs[1];
		  res[1] = _e[2]*rhs[0] - _e[0]*rhs[2];
		  res[2] = _e[0]*rhs[1] - _e[1]*rhs[0];
         
		  return res;
      }

//------------------------------------------------------------------------------
//!
      template< class T >
      inline Vec4<T> Vec4<T>::normal() const
      {
         T tmp = (T)1 / length();
         return Vec3<T>( _e[0] * tmp, _e[1] * tmp, _e[2] * tmp, _e[3] * tmp );
      }

//------------------------------------------------------------------------------
//!
      template< class T >
      inline Vec4<T>& Vec4<T>::normalEq() 
      {
         T tmp = (T)1 / length();
         _e[0] *= tmp;
         _e[1] *= tmp;
		 _e[2] *= tmp;
		 _e[3] *= tmp;
         return *this;
      }

//------------------------------------------------------------------------------
//!
      template< class T >
      inline Vec4<T>& Vec4<T>::normalize() 
      {
         T tmp = (T)1 / length();
         _e[0] *= tmp;
         _e[1] *= tmp;
		 _e[2] *= tmp;
		 _e[3] *= tmp;
         return *this;
      }
//------------------------------------------------------------------------------
//!
      template< class T >
      inline Vec4<T>& Vec4<T>::normalEq( const T len ) 
      {
         T tmp = len / length();
         _e[0] *= tmp;
         _e[1] *= tmp;
		 _e[2] *= tmp;
		 _e[3] *= tmp;
         return *this;
      }

//------------------------------------------------------------------------------
//!
      template< class T >
      inline Vec4<T>& Vec4<T>::negateEq()
      {
         _e[0] = -_e[0];
         _e[1] = -_e[1];
		 _e[2] = -_e[2];
		 _e[3] = -_e[3];
         return *this;
      }

//------------------------------------------------------------------------------
//!
      template< class T >
      inline Vec4<T>& Vec4<T>::clampToMaxEq( const T& max )
      {
         if( _e[0] > max )
         {
            _e[0] = max;
         }
         if( _e[1] > max )
         {
            _e[1] = max;
         }
		 if( _e[2] > max )
         {
            _e[2] = max;
         }
		 if( _e[3] > max )
		 {
			 _e[3] = max;
		 }
         return *this;
      }

//------------------------------------------------------------------------------
//!
      template< class T >
      inline Vec4<T> Vec4<T>::operator+( const Vec4<T>& rhs ) const
      {
         return Vec3<T>(
            _e[0] + rhs._e[0],
            _e[1] + rhs._e[1],
			_e[2] + rhs._e[2],
			_e[3] + rhs._e[3]
            );
      }

//------------------------------------------------------------------------------
//!
      template< class T >
      inline Vec4<T> Vec4<T>::operator-( const Vec4<T>& rhs ) const
      {
         return Vec3<T>(
            _e[0] - rhs._e[0],
            _e[1] - rhs._e[1],
			_e[2] - rhs._e[2],
			_e[3] - rhs._e[3]
            );
      }

//------------------------------------------------------------------------------
//!
      template< class T >
      inline Vec4<T> Vec4<T>::operator-() const
      {
         return Vec3<T>( -_e[0], -_e[1], -_e[2], -_e[3] );
      }

//------------------------------------------------------------------------------
//!
      template< class T >
      inline Vec4<T> Vec4<T>::operator*( const T& rhs ) const
      {
         return Vec3<T>( _e[0] * rhs, _e[1] * rhs, _e[2] * rhs, _e[3] * rhs );
      }

//------------------------------------------------------------------------------
//!
      template< class T >
      inline Vec4<T> Vec4<T>::operator*( const Vec4<T>& rhs ) const
      {
         return Vec3<T>( _e[0] * rhs._e[0], _e[1] * rhs._e[1], _e[2] * rhs._e[2], _e[3] * rhs._e[3] );
      }

//------------------------------------------------------------------------------
//!
      template< class T >
      inline Vec4<T> Vec4<T>::operator/( const T& rhs ) const
      {
         return Vec3<T>( _e[0] / rhs, _e[1] / rhs, _e[2] / rhs, _e[3] / rhs );
      }

//------------------------------------------------------------------------------
//!
      template< class T >
      inline Vec4<T> Vec4<T>::operator/( const Vec4<T>& rhs ) const
      {
         return Vec3<T>( _e[0] / rhs._e[0], _e[1] / rhs._e[1], _e[2] / rhs._e[2], _e[3] / rhs._e[3] );
      }

//------------------------------------------------------------------------------
//!
      template< class T >
      inline Vec4<T>& Vec4<T>::operator+=( const Vec4<T>& rhs )
      {
         _e[0] += rhs._e[0];
         _e[1] += rhs._e[1];
		 _e[2] += rhs._e[2];
		 _e[3] += rhs._e[3];
         return *this;
      }

//------------------------------------------------------------------------------
//!
      template< class T >
      inline Vec4<T>& Vec4<T>::operator-=( const Vec4<T>& rhs )
      {
         _e[0] -= rhs._e[0];
         _e[1] -= rhs._e[1];
		 _e[2] -= rhs._e[2];
		 _e[3] -= rhs._e[3];
         return *this;
      }

//------------------------------------------------------------------------------
//!
      template< class T >
      inline Vec4<T>& Vec4<T>::operator*=( const T& rhs )
      {
         _e[0] *= rhs;
         _e[1] *= rhs;
		 _e[2] *= rhs;
		 _e[3] *= rhs;
         return *this;
      }

//------------------------------------------------------------------------------
//!
      template< class T >
      inline Vec4<T>& Vec4<T>::operator*=( const Vec4<T>& rhs )
      {
         _e[0] *= rhs._e[0];
         _e[1] *= rhs._e[1];
		 _e[2] *= rhs._e[2];
		 _e[3] *= rhs._e[3];
         return *this;
      }

//------------------------------------------------------------------------------
//!
      template< class T >
      inline Vec4<T>& Vec4<T>::operator/=( const T& rhs )
      {
         _e[0] /= rhs;
         _e[1] /= rhs;
		 _e[2] /= rhs;
		 _e[3] /= rhs;
         return *this;
      }

//------------------------------------------------------------------------------
//!
      template< class T >
      inline Vec4<T>& Vec4<T>::operator/=( const Vec4<T>& rhs )
      {
         _e[0] /= rhs._e[0];
         _e[1] /= rhs._e[1];
		 _e[2] /= rhs._e[2];
		 _e[3] /= rhs._e[3];
         return *this;
      }

//------------------------------------------------------------------------------
//!
      template< class T >
      inline Vec4<T>& Vec4<T>::operator=( const Vec4<T>& rhs )
      {
         _e[0] = rhs._e[0];
         _e[1] = rhs._e[1];
		 _e[2] = rhs._e[2];
		 _e[3] = rhs._e[3];
         return *this;
      }

//------------------------------------------------------------------------------
//!
      template< class T >
      inline bool Vec4<T>::operator==( const Vec4<T>& rhs ) const
      {
         return _e[0] == rhs._e[0] && _e[1] == rhs._e[1] && _e[2] == rhs._e[2]  && _e[3] == rhs._e[3]; 
      }

//------------------------------------------------------------------------------
//!
      template< class T >
      inline bool Vec4<T>::operator!=( const Vec4<T>& rhs ) const
      {
         return _e[0] != rhs._e[0] || _e[1] != rhs._e[1] || _e[2] != rhs._e[2] || _e[3] != rhs._e[3];
      }

//------------------------------------------------------------------------------
//!
      template< class T >
      inline T& Vec4<T>::operator()( int idx )
      {
         return _e[idx];
      }

//------------------------------------------------------------------------------
//!
      template< class T >
      inline const T& Vec4<T>::operator()
         ( int idx ) const
      {
         return _e[idx];
      }

//------------------------------------------------------------------------------
//!
      template< class T >
      inline T& Vec4<T>::operator[]( int idx )
      {
         return _e[idx];
      }

//------------------------------------------------------------------------------
//!
      template< class T >
      inline const T& Vec4<T>::operator[]( int idx ) const
      {
         return _e[idx];
      }

//------------------------------------------------------------------------------
//!
      template< class T >
      void
      Vec4<T>::set( T const x, T const y, T const z, T const w )
      {
         _e[0] = x;
         _e[1] = y;
		 _e[2] = z;
		 _e[3] = w;
      }
      
      
      
//------------------------------------------------------------------------------
//!
      template< class T >
      inline T& Vec4<T>::x() {
         return _e[0];
      }

//------------------------------------------------------------------------------
//!
      template< class T >
      inline T& Vec4<T>::y() {
         return _e[1];
      }

//------------------------------------------------------------------------------
//!
      template< class T >
      inline T& Vec4<T>::z() {
         return _e[2];
      }

	  //------------------------------------------------------------------------------
	  //!
	  template< class T >
	  inline T& Vec4<T>::w() {
		  return _e[3];
	  }

//------------------------------------------------------------------------------
//!
      template< class T >
      inline const T& Vec4<T>::x() const {
         return _e[0];
      }

//------------------------------------------------------------------------------
//!
      template< class T >
      inline const T& Vec4<T>::y() const {
         return _e[1];
      }

//------------------------------------------------------------------------------
//!
      template< class T >
      inline const T& Vec4<T>::z() const {
         return _e[2];
      }

	  //------------------------------------------------------------------------------
	  //!
	  template< class T >
	  inline const T& Vec4<T>::w() const {
		  return _e[3];
	  }

//------------------------------------------------------------------------------
//!
      template< class T >
      inline void Vec4<T>::setX(const T & x) {
         _e[0] = x;
      }

//------------------------------------------------------------------------------
//!
      template< class T >
      inline void Vec4<T>::setY(const T & y) {
         _e[1] = y;
      }

//------------------------------------------------------------------------------
//!
      template< class T >
      inline void Vec4<T>::setZ(const T & z) {
         _e[2] = z;
      }

	  //------------------------------------------------------------------------------
	  //!
	  template< class T >
	  inline void Vec4<T>::setW(const T & w) {
		  _e[3] = w;
	  }

//------------------------------------------------------------------------------
//!
	  template< class T >
      inline Vec4<T> operator*( const T& val, const Vec4<T>& vec )
      {
         return Vec4<T>( vec(0) * val, vec(1) * val, vec(2) * val, vec(3) * val );
      }

/*==============================================================================
  TYPEDEF
  ==============================================================================*/

      typedef Vec4< int >    Vec4mi;
      typedef Vec4< float >  Vec4mf;
      typedef Vec4< double > Vec4md;

	  typedef Vec4 < float >  TrajectoryPoint;
	  typedef Vec4 < float >  Color4D;
	  typedef Vec4 < unsigned char > Color4B;

      //External Operator
      template< class T >
      inline std::ostream&
      operator<<(std::ostream & os, Vec4<T> const & v)
      {
         return os << " [ " << v.x() << "; " << v.y() << "; " << v.z() << "; " << v.w() << " ] " << std::endl;
      }

#endif // Vec4_h__