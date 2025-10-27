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

            CEArray ( std::initializer_list<T> initList )
                : DataPtr ( nullptr ), ArraySize ( initList.size () ), ArrayCapacity ( initList.size () )
                {
                if (ArraySize > 0)
                    {
                    DataPtr = static_cast< T * >( std::malloc ( ArraySize * sizeof ( T ) ) );
                    size_t i = 0;
                    for (const auto & item : initList)
                        {
                        new( &DataPtr[ i ] ) T ( item );
                        i++;
                        }
                    }
                }

                // Конструктор для C-массивов
            template<size_t N>
            CEArray ( const T ( &array )[ N ] )
                : DataPtr ( nullptr ), ArraySize ( N ), ArrayCapacity ( N )
                {
                if (ArraySize > 0)
                    {
                    DataPtr = static_cast< T * >( std::malloc ( ArraySize * sizeof ( T ) ) );
                    for (size_t i = 0; i < ArraySize; i++)
                        {
                        new( &DataPtr[ i ] ) T ( array[ i ] );
                        }
                    }
                }

                // Capacity
            bool IsEmpty () const { return ArraySize == 0; }
            uint64 Size () const { return ArraySize; }
            uint64 Capacity () const { return ArrayCapacity; }

            size_t size () const { return static_cast< size_t >( ArraySize ); }
            size_t capacity () const { return static_cast< size_t >( ArrayCapacity ); }
            bool empty () const { return ArraySize == 0; }

            T * RawData () { return DataPtr; }
            const T * RawData () const { return DataPtr; }

            T & operator[]( uint64 index )
                {
                    // ВРЕМЕННОЕ ИСПРАВЛЕНИЕ:
                if (index >= ArraySize)
                    {
                    CE_CORE_ERROR ( "CEArray INDEX OUT OF BOUNDS: index={}, size={}, capacity={}",
                                    index, ArraySize, ArrayCapacity );

                       // Если массив пустой, создадим временный элемент
                    if (ArraySize == 0 && ArrayCapacity > 0)
                        {
                        CE_CORE_WARN ( "Array is empty but accessed with index {}. Creating temporary element.", index );
                        new ( DataPtr ) T ();  // создаем элемент по умолчанию
                        ArraySize = 1;
                        }
                    else
                        {
                             // Вернем последний валидный элемент
                        index = ArraySize > 0 ? ArraySize - 1 : 0;
                        }
                    }

                    //CE_CORE_ASSERT(index < ArraySize, "CEArray index out of bounds!");
                return DataPtr[ index ];
                }

            const T & operator[]( uint64 index ) const
                {
                    // Аналогично для const версии
                if (index >= ArraySize)
                    {
                    CE_CORE_ERROR ( "CEArray INDEX OUT OF BOUNDS (const): index={}, size={}", index, ArraySize );
                    static T defaultVal;
                    return defaultVal;
                    }
                    //CE_CORE_ASSERT(index < ArraySize, "CEArray index out of bounds!");
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