// Runtime/Core/Containers/CEArray.hpp
#pragma once
#include "Core/CoreTypes.hpp"
#include <algorithm>
#include <memory>
#include <cstdlib>

#ifndef CE_CORE_ASSERT
#define CE_CORE_ASSERT(condition, message) \
    do { if (!(condition)) { /* можно добавить debug break */ } } while(0)
#endif

namespace CE
    {
    template<typename T>
    class CEArray
        {
        public:
          
            CEArray () : DataPtr ( nullptr ), ArraySize ( 0 ), ArrayCapacity ( 0 ) { }

            explicit CEArray ( uint64 initialCapacity )
                : DataPtr ( nullptr ), ArraySize ( 0 ), ArrayCapacity ( 0 )
                {
                if (initialCapacity > 0)
                    Reserve ( initialCapacity );
                }

            ~CEArray ()
                {
                Clear ();
                if (DataPtr)
                    std::free ( DataPtr );
                }

                
            CEArray ( CEArray && other ) noexcept
                : DataPtr ( other.DataPtr ), ArraySize ( other.ArraySize ), ArrayCapacity ( other.ArrayCapacity )
                {
                other.DataPtr = nullptr;
                other.ArraySize = 0;
                other.ArrayCapacity = 0;
                }

            CEArray & operator=( CEArray && other ) noexcept
                {
                if (this != &other)
                    {
                    Clear ();
                    std::free ( DataPtr );

                    DataPtr = other.DataPtr;
                    ArraySize = other.ArraySize;
                    ArrayCapacity = other.ArrayCapacity;

                    other.DataPtr = nullptr;
                    other.ArraySize = 0;
                    other.ArrayCapacity = 0;
                    }
                return *this;
                }

                // Capacity
            bool IsEmpty () const { return ArraySize == 0; }
            uint64 Size () const { return ArraySize; }
            uint64 Capacity () const { return ArrayCapacity; }

           
            T * RawData () { return DataPtr; }
            const T * RawData () const { return DataPtr; }

            // Element access
            T & operator[]( uint64 index )
                {
                CE_CORE_ASSERT ( index < ArraySize, "CEArray index out of bounds!" );
                return DataPtr[ index ];
                }

            const T & operator[]( uint64 index ) const
                {
                CE_CORE_ASSERT ( index < ArraySize, "CEArray index out of bounds!" );
                return DataPtr[ index ];
                }

            T & Front () { return DataPtr[ 0 ]; }
            const T & Front () const { return DataPtr[ 0 ]; }
            T & Back () { return DataPtr[ ArraySize - 1 ]; }
            const T & Back () const { return DataPtr[ ArraySize - 1 ]; }

            // Memory management
            void Reserve ( uint64 newCapacity )
                {
                if (newCapacity <= ArrayCapacity) return;

                T * newData = static_cast< T * >( std::malloc ( newCapacity * sizeof ( T ) ) );
              
                if (!newData) return;

                // Move existing elements
                for (uint64 i = 0; i < ArraySize; ++i)
                    {
                    new ( newData + i ) T ( std::move ( DataPtr[ i ] ) );
                    DataPtr[ i ].~T ();
                    }

                std::free ( DataPtr );
                DataPtr = newData;
                ArrayCapacity = newCapacity;
                }

            void Resize ( uint64 newSize )
                {
                if (newSize > ArrayCapacity)
                    Reserve ( newSize );

                // Construct new elements
                for (uint64 i = ArraySize; i < newSize; ++i)
                    new ( DataPtr + i ) T ();

                // Destroy extra elements
                for (uint64 i = newSize; i < ArraySize; ++i)
                    DataPtr[ i ].~T ();

                ArraySize = newSize;
                }

                // Modifiers
            void PushBack ( const T & value )
                {
                if (ArraySize >= ArrayCapacity)
                    Reserve ( ArrayCapacity == 0 ? 4 : ArrayCapacity * 2 );

                new ( DataPtr + ArraySize ) T ( value );
                ++ArraySize;
                }

            void PushBack ( T && value )
                {
                if (ArraySize >= ArrayCapacity)
                    Reserve ( ArrayCapacity == 0 ? 4 : ArrayCapacity * 2 );

                new ( DataPtr + ArraySize ) T ( std::move ( value ) );
                ++ArraySize;
                }

            template<typename... Args>
            T & EmplaceBack ( Args&&... args )
                {
                if (ArraySize >= ArrayCapacity)
                    Reserve ( ArrayCapacity == 0 ? 4 : ArrayCapacity * 2 );

                T * ptr = new ( DataPtr + ArraySize ) T ( std::forward<Args> ( args )... );
                ++ArraySize;
                return *ptr;
                }

            void PopBack ()
                {
                if (ArraySize > 0)
                    {
                    DataPtr[ ArraySize - 1 ].~T ();
                    --ArraySize;
                    }
                }

            void Clear ()
                {
                for (uint64 i = 0; i < ArraySize; ++i)
                    DataPtr[ i ].~T ();
                ArraySize = 0;
                }

                // Iterators
            T * begin () { return DataPtr; }
            T * end () { return DataPtr + ArraySize; }
            const T * begin () const { return DataPtr; }
            const T * end () const { return DataPtr + ArraySize; }

        private:
            T * DataPtr = nullptr;  
            uint64 ArraySize = 0;
            uint64 ArrayCapacity = 0;

           
            CEArray ( const CEArray & ) = delete;
            CEArray & operator=( const CEArray & ) = delete;
        };
    }