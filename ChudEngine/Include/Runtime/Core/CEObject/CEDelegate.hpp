#pragma once
#include "Core/CEObject/CEObject.hpp"
#include <functional>
#include <vector>
#include <memory>

namespace CE
    {
        // ������� ����� ��������
    class CEDelegateBase : public CEObject
        {
        public:
            virtual ~CEDelegateBase () = default;
            virtual void Clear () = 0;
            virtual size_t GetNumBindings () const = 0;
        };

        // ��������������� �������
    template<typename T>
    class CEDelegate : public CEDelegateBase
        {
        public:
            using FunctionType = std::function<void ( T )>;

            CEDelegate ()
                {
                SetName ( "CEDelegate" );
                }

                // �������� �������
            void Bind ( FunctionType Function )
                {
                Functions.push_back ( Function );
                CE_DEBUG ( "Delegate '{}' bound new function (total: {})", GetName (), Functions.size () );
                }

                // �������� ������ �������
            template<typename ClassType>
            void Bind ( ClassType * Object, void ( ClassType:: * Method )( T ) )
                {
                Functions.push_back ( [ Object, Method ] ( T Arg )
                                      {
                                      if (Object) ( Object->*Method )( Arg );
                                      } );
                CE_DEBUG ( "Delegate '{}' bound object method (total: {})", GetName (), Functions.size () );
                }

                // ����� ��������
            void Broadcast ( T Arg )
                {
                if (Functions.empty ()) return;

                for (auto & Function : Functions)
                    {
                    if (Function) Function ( Arg );
                    }

                CE_DEBUG ( "Delegate '{}' broadcast to {} functions", GetName (), Functions.size () );
                }

                // ������� ��������
            virtual void Clear () override
                {
                Functions.clear ();
                CE_DEBUG ( "Delegate '{}' cleared all bindings", GetName () );
                }

                // ���������� ��������
            virtual size_t GetNumBindings () const override
                {
                return Functions.size ();
                }

        private:
            std::vector<FunctionType> Functions;
        };

        // ������������� ������� (��� ����������)
    class CEMulticastDelegate : public CEDelegateBase
        {
        public:
            using FunctionType = std::function<void ()>;

            CEMulticastDelegate ()
                {
                SetName ( "CEMulticastDelegate" );
                }

                // ���������� �������
            void Add ( FunctionType Function )
                {
                Functions.push_back ( Function );
                CE_DEBUG ( "MulticastDelegate '{}' added function (total: {})", GetName (), Functions.size () );
                }

                // ���������� ������ �������
            template<typename ClassType>
            void Add ( ClassType * Object, void ( ClassType:: * Method )( ) )
                {
                Functions.push_back ( [ Object, Method ] ()
                                      {
                                      if (Object) ( Object->*Method )( );
                                      } );
                CE_DEBUG ( "MulticastDelegate '{}' added object method (total: {})", GetName (), Functions.size () );
                }

                // ����� ���� �������
            void Broadcast ()
                {
                if (Functions.empty ()) return;

                for (auto & Function : Functions)
                    {
                    if (Function) Function ();
                    }

                CE_DEBUG ( "MulticastDelegate '{}' broadcast to {} functions", GetName (), Functions.size () );
                }

            virtual void Clear () override
                {
                Functions.clear ();
                CE_DEBUG ( "MulticastDelegate '{}' cleared all bindings", GetName () );
                }

            virtual size_t GetNumBindings () const override
                {
                return Functions.size ();
                }

        private:
            std::vector<FunctionType> Functions;
        };

        // ������������� ������� � ����� ����������
    template<typename T>
    class CEMulticastDelegate1 : public CEDelegateBase
        {
        public:
            using FunctionType = std::function<void ( T )>;

            CEMulticastDelegate1 ()
                {
                SetName ( "CEMulticastDelegate1" );
                }

            void Add ( FunctionType Function )
                {
                Functions.push_back ( Function );
                CE_DEBUG ( "MulticastDelegate1 '{}' added function (total: {})", GetName (), Functions.size () );
                }

            template<typename ClassType>
            void Add ( ClassType * Object, void ( ClassType:: * Method )( T ) )
                {
                Functions.push_back ( [ Object, Method ] ( T Arg )
                                      {
                                      if (Object) ( Object->*Method )( Arg );
                                      } );
                CE_DEBUG ( "MulticastDelegate1 '{}' added object method (total: {})", GetName (), Functions.size () );
                }

            void Broadcast ( T Arg )
                {
                if (Functions.empty ()) return;

                for (auto & Function : Functions)
                    {
                    if (Function) Function ( Arg );
                    }

                CE_DEBUG ( "MulticastDelegate1 '{}' broadcast to {} functions", GetName (), Functions.size () );
                }

            virtual void Clear () override
                {
                Functions.clear ();
                CE_DEBUG ( "MulticastDelegate1 '{}' cleared all bindings", GetName () );
                }

            virtual size_t GetNumBindings () const override
                {
                return Functions.size ();
                }

        private:
            std::vector<FunctionType> Functions;
        };
    }