/**************************************************************
 * 
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 * 
 *   http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 * 
 *************************************************************/



#ifndef _UNIVERSALL_REFERENCE_HXX
#define _UNIVERSALL_REFERENCE_HXX

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif
#ifndef _OSL_INTERLOCK_H_
#include <osl/interlck.h>
#endif
namespace binfilter {

/**
 * An instance of this class holds a pointer to an object. The lifetime of 
 * the object is controled by the instance. The constructor calls
 * acquire() and the destructor calls release().
 * You could delive your class from the baseclass UniRefBase wich implements
 * the methods acquire and release, yet.
 */
template< class T > class UniReference
{
private:
	T*	mpElement;

public:
	/** Create an empty reference.*/
	UniReference()
	: mpElement( NULL )
	{}

	/** Destroy the reference and releases the element.*/
	inline ~UniReference();

	/** Create a new reference with the same element as in rRef and acquire this one.*/
	inline UniReference( const UniReference< T > & rRef );

	/** 
	 * Create a new reference with the given element pElement and acquire this one.
	 */
	inline UniReference( T * pElement );

	/**
	 * Release the reference and set the new one pObj.
	 */
    inline UniReference< T > & operator = ( T * pElement );

	/**
	 * Release the reference and set the new one from rObj.
	 */
    inline UniReference< T > & operator = ( const UniReference< T > & rRef );

	/**
	 * Return the pointer to the element, may be null.
	 */
    inline T* operator -> () const;

	/**
	 * Returns true if the pointer to the element is valid.
	 */
	inline sal_Bool is() const;

	/**
	 * Return true if both elements refer to the same object.
	 */
	inline sal_Bool operator == ( const UniReference & rRef ) const;

	/**
	 * Return true if both elements does not refer to the same object.
	 */
	inline sal_Bool operator != ( const UniReference & rRef ) const;

	/** Gets implementation pointer.
		This call does <b>not</b> acquire the implementation.
		<br>
		@return <b>un</b>acquired implementation pointer
	*/
    inline T* get() const;
};

class UniRefBase
{
private:
	/**
	 * The reference counter.
	 */
    oslInterlockedCount			m_refCount;

public:
	UniRefBase() : m_refCount( 0 )
	{}
	virtual ~UniRefBase();

    void acquire() { osl_incrementInterlockedCount( &m_refCount ); }
    void release();
};

///////////////////////////////////////////////////////////////////////////////
//
// Inline-implementations of UniReference
//

/** Create a new reference with the same element as in rRef and acquire this one.*/
template< class T >
inline UniReference< T >::UniReference( const UniReference< T > & rRef )
	: mpElement( rRef.mpElement )
{
	if( mpElement )
		mpElement->acquire();
}

template< class T >
inline UniReference< T >::~UniReference()
{
	if( mpElement )
		mpElement->release();
}

/** 
 * Create a new reference with the given element pElement and acquire this one.
 * @param pInterface the interface, pointer may be null.
 */
template< class T >
inline UniReference< T >::UniReference( T * pElement )
	: mpElement( pElement )
{
	if( mpElement )
		mpElement->acquire();
}

/**
 * Release the reference and set the new one pObj.<BR>
 * <B>The operation is not thread save. You must protect all assigns to a reference class.</B>
 */
template< class T >
inline UniReference< T > & UniReference< T >::operator = ( T * pElement )
{
	if( pElement )
		pElement->acquire();
	if( mpElement )
		mpElement->release();

	mpElement = pElement;

	return *this;
}

/**
 * Release the reference and set the new one from rObj.<BR>
 * <B>The operation is not thread save. You must protect all assigns to a reference class.</B>
 */
template< class T >
inline UniReference< T > & UniReference< T >::operator = ( const UniReference< T > & rRef )
{
	return operator = ( rRef.mpElement );
}

/**
 * Return the pointer to the interface, may be null.
 */
template< class T >
inline T* UniReference< T >::operator -> () const 
{ 
	return get(); 
}

/**
 * Return the pointer to the interface, may be null.
 */
template< class T >
inline T* UniReference< T >::get () const 
{ 
	return static_cast< T * >( mpElement ); 
}

/**
 * Returns true if the pointer to the interface is valid.
 */
template< class T >
inline sal_Bool UniReference< T >::is() const
{ 
	return (mpElement != 0); 
}
/**
 * Return true if both interfaces refer to the same object. The operation can be
 * much more expensive than a pointer comparision.<BR>
 *
 * @param rRef		another interface reference
 */
template< class T >
inline sal_Bool UniReference< T >::operator == ( const UniReference & rRef ) const
{
	return ( mpElement == rRef.mpElement );
}
/**
 * Return true if both interfaces does not refer to the same object. The operation can be
 * much more expensive than a pointer comparision.<BR>
 *
 * @param rRef		another interface reference
 */
template< class T >
inline sal_Bool UniReference< T >::operator != ( const UniReference & rRef ) const
{ 
	return ( ! operator == ( rRef ) ); 
}

}//end of namespace binfilter
#endif	// _UNIVERSALL_REFERENCE_HXX
