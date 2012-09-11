/**
 *  Copyright (C) 2012, BMW AG
 *
 *  \author Christian Mueller, christian.ei.mueller@bmw.de BMW 2011,2012
 *
 *  \copyright
 *  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction,
 *  including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so,
 *  subject to the following conditions:
 *  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 *  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR
 *  THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 *  \file CAmSerializer.h
 *  For further information see http://www.genivi.org/.
 */

#ifndef CAMSERIALIZER_H_
#define CAMSERIALIZER_H_

#include <pthread.h>
#include <deque>
#include <cassert>
#include <memory>
#include <stdexcept>
#include <unistd.h>
#include "CAmDltWrapper.h"
#include "CAmSocketHandler.h"

/**
 * todo: performance improvement we could implement a memory pool that is more efficient here and avoids
 * allocation and deallocation times.
 */

namespace am
{
/**
 * magic class that does the serialization of functions calls
 * The constructor must be called within the main threadcontext, after that using the
 * overloaded template function call will serialize all calls and call them within the
 * main thread context.\n
 * More details can be found here: \ref util
 * \warning asynchronous calls may be used in the mainthread context, but if you want to use synchronous calls make sure that you use one
 * instance of this class per thread otherwise you could be lost in never returning calls.\n
 * Examples of the usage can be found in IAmCommandReceiverShadow of the ControlPlugin or IAmRoutingReceiverShadow of the
 * PluginRoutingInterfaceAsync.
 *
 */
class CAmSerializer
{
private:

    /**
     * Prototype for a delegate
     */
    class CAmDelegate
    {
    public:
        virtual ~CAmDelegate()
        {};
        virtual bool call(int* pipe)=0;

    };

    typedef CAmDelegate* CAmDelegagePtr; //!< pointer to a delegate

    /**
     * delegate template for no argument
     */
    template<class TClass> class CAmNoArgDelegate: public CAmDelegate
    {
    private:
        TClass* mInstance;
        void (TClass::*mFunction)();

    public:
        CAmNoArgDelegate(TClass* instance, void (TClass::*function)()) :
                mInstance(instance), //
                mFunction(function)
        {};

        bool call(int* pipe)
        {
            (void) pipe;
            (*mInstance.*mFunction)();
            return (true);
        };
    };

    /**
     * delegate template for one argument
     */
    template<class TClass, typename Targ> class CAmOneArgDelegate: public CAmDelegate
    {
    private:
        TClass* mInstance;
        void (TClass::*mFunction)(Targ);
        Targ mArgument;

    public:
        CAmOneArgDelegate(TClass* instance, void (TClass::*function)(Targ), Targ argument) :
                mInstance(instance), //
                mFunction(function), //
                mArgument(argument)
        {};

        bool call(int* pipe)
        {
            (void) pipe;
            (*mInstance.*mFunction)(mArgument);
            return (true);
        };
    };

    /**
     * delegate template for two arguments
     */
    template<class TClass, typename Targ, typename Targ1> class CAmTwoArgDelegate: public CAmDelegate
    {
    private:
        TClass* mInstance;
        void (TClass::*mFunction)(Targ argument, Targ1 argument1);
        Targ mArgument;
        Targ1 mArgument1;

    public:
        CAmTwoArgDelegate(TClass* instance, void (TClass::*function)(Targ argument, Targ1 argument1), Targ argument, Targ1 argument1) :
                mInstance(instance), //
                mFunction(function), //
                mArgument(argument), //
                mArgument1(argument1)
        { };

        bool call(int* pipe)
        {
            (void) pipe;
            (*mInstance.*mFunction)(mArgument, mArgument1);
            return (true);
        };
    };

    /**
     * delegate template for two arguments
     */
    template<class TClass, typename Targ, typename Targ1> class CAmTwoArgDelegateFirstRef: public CAmDelegate
    {
    private:
        TClass* mInstance;
        void (TClass::*mFunction)(Targ& argument, Targ1 argument1);
        Targ mArgument;
        Targ1 mArgument1;

    public:
        CAmTwoArgDelegateFirstRef(TClass* instance, void (TClass::*function)(Targ& argument, Targ1 argument1), Targ& argument, Targ1 argument1) :
                mInstance(instance), //
                mFunction(function), //
                mArgument(argument), //
                mArgument1(argument1)
        { };

        bool call(int* pipe)
        {
            (void) pipe;
            (*mInstance.*mFunction)(mArgument, mArgument1);
            return (true);
        };
    };


    template<class TClass, typename Targ, typename Targ1> class CAmTwoArgDelegateSecondRef: public CAmDelegate
    {
    private:
        TClass* mInstance;
        void (TClass::*mFunction)(Targ argument, Targ1& argument1);
        Targ mArgument;
        Targ1 mArgument1;

    public:
        CAmTwoArgDelegateSecondRef(TClass* instance, void (TClass::*function)(Targ argument, Targ1& argument1), Targ argument, Targ1& argument1) :
                mInstance(instance), //
                mFunction(function), //
                mArgument(argument), //
                mArgument1(argument1)
        {};

        bool call(int* pipe)
        {
            (void) pipe;
            (*mInstance.*mFunction)(mArgument, mArgument1);
            return (true);
        };
    };

    /**
     * delegate template for two arguments
     */
    template<class TClass, typename Targ, typename Targ1> class CAmTwoArgDelegateAllRef: public CAmDelegate
    {
    private:
        TClass* mInstance;
        void (TClass::*mFunction)(Targ& argument, Targ1& argument1);
        Targ mArgument;
        Targ1 mArgument1;

    public:
        CAmTwoArgDelegateAllRef(TClass* instance, void (TClass::*function)(Targ& argument, Targ1& argument1), Targ& argument, Targ1& argument1) :
                mInstance(instance), //
                mFunction(function), //
                mArgument(argument), //
                mArgument1(argument1)
        { };

        bool call(int* pipe)
        {
            (void) pipe;
            (*mInstance.*mFunction)(mArgument, mArgument1);
            return (true);
        };
    };


    /**
     * delegate template for three arguments
     */
    template<class TClass, typename Targ, typename Targ1, typename Targ2> class CAmThreeArgDelegate: public CAmDelegate
    {
    private:
        TClass* mInstance;
        void (TClass::*mFunction)(Targ argument, Targ1 argument1, Targ2 argument2);
        Targ mArgument;
        Targ1 mArgument1;
        Targ2 mArgument2;

    public:
        CAmThreeArgDelegate(TClass* instance, void (TClass::*function)(Targ argument, Targ1 argument1, Targ2 argument2), Targ argument, Targ1 argument1, Targ2 argument2) :
                mInstance(instance), //
                mFunction(function), //
                mArgument(argument), //
                mArgument1(argument1), //
                mArgument2(argument2)
        {
        }
        ;

        bool call(int* pipe)
        {
            (void) pipe;
            (*mInstance.*mFunction)(mArgument, mArgument1, mArgument2);
            return (true);
        }
        ;
    };

    /**
      * delegate template for three arguments
      */
     template<class TClass, typename Targ, typename Targ1, typename Targ2> class CAmThreeArgDelegateFirstRef: public CAmDelegate
     {
     private:
         TClass* mInstance;
         void (TClass::*mFunction)(Targ& argument, Targ1 argument1, Targ2 argument2);
         Targ mArgument;
         Targ1 mArgument1;
         Targ2 mArgument2;

     public:
         CAmThreeArgDelegateFirstRef(TClass* instance, void (TClass::*function)(Targ& argument, Targ1 argument1, Targ2 argument2), Targ& argument, Targ1 argument1, Targ2 argument2) :
                 mInstance(instance), //
                 mFunction(function), //
                 mArgument(argument), //
                 mArgument1(argument1), //
                 mArgument2(argument2)
         {};

         bool call(int* pipe)
         {
             (void) pipe;
             (*mInstance.*mFunction)(mArgument, mArgument1, mArgument2);
             return (true);
         };
     };

     /**
       * delegate template for three arguments
       */
      template<class TClass, typename Targ, typename Targ1, typename Targ2> class CAmThreeArgDelegateSecondRef: public CAmDelegate
      {
      private:
          TClass* mInstance;
          void (TClass::*mFunction)(Targ argument, Targ1& argument1, Targ2 argument2);
          Targ mArgument;
          Targ1 mArgument1;
          Targ2 mArgument2;

      public:
          CAmThreeArgDelegateSecondRef(TClass* instance, void (TClass::*function)(Targ argument, Targ1& argument1, Targ2 argument2), Targ argument, Targ1& argument1, Targ2 argument2) :
                  mInstance(instance), //
                  mFunction(function), //
                  mArgument(argument), //
                  mArgument1(argument1), //
                  mArgument2(argument2)
          {};

          bool call(int* pipe)
          {
              (void) pipe;
              (*mInstance.*mFunction)(mArgument, mArgument1, mArgument2);
              return (true);
          };
      };

      /**
        * delegate template for three arguments
        */
       template<class TClass, typename Targ, typename Targ1, typename Targ2> class CAmThreeArgDelegateThirdRef: public CAmDelegate
       {
       private:
           TClass* mInstance;
           void (TClass::*mFunction)(Targ argument, Targ1 argument1, Targ2& argument2);
           Targ mArgument;
           Targ1 mArgument1;
           Targ2 mArgument2;

       public:
           CAmThreeArgDelegateThirdRef(TClass* instance, void (TClass::*function)(Targ argument, Targ1 argument1, Targ2& argument2), Targ argument, Targ1 argument1, Targ2& argument2) :
                   mInstance(instance), //
                   mFunction(function), //
                   mArgument(argument), //
                   mArgument1(argument1), //
                   mArgument2(argument2)
           {};

           bool call(int* pipe)
           {
               (void) pipe;
               (*mInstance.*mFunction)(mArgument, mArgument1, mArgument2);
               return (true);
           };
       };

       /**
         * delegate template for three arguments
         */
        template<class TClass, typename Targ, typename Targ1, typename Targ2> class CAmThreeArgDelegateFirstSecondRef: public CAmDelegate
        {
        private:
            TClass* mInstance;
            void (TClass::*mFunction)(Targ& argument, Targ1& argument1, Targ2 argument2);
            Targ mArgument;
            Targ1 mArgument1;
            Targ2 mArgument2;

        public:
            CAmThreeArgDelegateFirstSecondRef(TClass* instance, void (TClass::*function)(Targ& argument, Targ1& argument1, Targ2 argument2), Targ& argument, Targ1& argument1, Targ2 argument2) :
                    mInstance(instance), //
                    mFunction(function), //
                    mArgument(argument), //
                    mArgument1(argument1), //
                    mArgument2(argument2)
            {};

            bool call(int* pipe)
            {
                (void) pipe;
                (*mInstance.*mFunction)(mArgument, mArgument1, mArgument2);
                return (true);
            };
        };

        /**
          * delegate template for three arguments
          */
         template<class TClass, typename Targ, typename Targ1, typename Targ2> class CAmThreeArgDelegateFirstThirdRef: public CAmDelegate
         {
         private:
             TClass* mInstance;
             void (TClass::*mFunction)(Targ& argument, Targ1 argument1, Targ2& argument2);
             Targ mArgument;
             Targ1 mArgument1;
             Targ2 mArgument2;

         public:
             CAmThreeArgDelegateFirstThirdRef(TClass* instance, void (TClass::*function)(Targ& argument, Targ1 argument1, Targ2& argument2), Targ& argument, Targ1 argument1, Targ2& argument2) :
                     mInstance(instance), //
                     mFunction(function), //
                     mArgument(argument), //
                     mArgument1(argument1), //
                     mArgument2(argument2)
             {};

             bool call(int* pipe)
             {
                 (void) pipe;
                 (*mInstance.*mFunction)(mArgument, mArgument1, mArgument2);
                 return (true);
             };
         };

         /**
           * delegate template for three arguments
           */
          template<class TClass, typename Targ, typename Targ1, typename Targ2> class CAmThreeArgDelegateSecondThirdRef: public CAmDelegate
          {
          private:
              TClass* mInstance;
              void (TClass::*mFunction)(Targ argument, Targ1& argument1, Targ2& argument2);
              Targ mArgument;
              Targ1 mArgument1;
              Targ2 mArgument2;

          public:
              CAmThreeArgDelegateSecondThirdRef(TClass* instance, void (TClass::*function)(Targ argument, Targ1& argument1, Targ2& argument2), Targ argument, Targ1& argument1, Targ2& argument2) :
                      mInstance(instance), //
                      mFunction(function), //
                      mArgument(argument), //
                      mArgument1(argument1), //
                      mArgument2(argument2)
              {};

              bool call(int* pipe)
              {
                  (void) pipe;
                  (*mInstance.*mFunction)(mArgument, mArgument1, mArgument2);
                  return (true);
              };
          };

          /**
            * delegate template for three arguments
            */
           template<class TClass, typename Targ, typename Targ1, typename Targ2> class CAmThreeArgDelegateAllRef: public CAmDelegate
           {
           private:
               TClass* mInstance;
               void (TClass::*mFunction)(Targ& argument, Targ1& argument1, Targ2& argument2);
               Targ mArgument;
               Targ1 mArgument1;
               Targ2 mArgument2;

           public:
               CAmThreeArgDelegateAllRef(TClass* instance, void (TClass::*function)(Targ& argument, Targ1& argument1, Targ2& argument2), Targ& argument, Targ1& argument1, Targ2& argument2) :
                       mInstance(instance), //
                       mFunction(function), //
                       mArgument(argument), //
                       mArgument1(argument1), //
                       mArgument2(argument2)
               {};

               bool call(int* pipe)
               {
                   (void) pipe;
                   (*mInstance.*mFunction)(mArgument, mArgument1, mArgument2);
                   return (true);
               };
           };

    /**
     * delegate template for four arguments
     */
    template<class TClass, typename Targ, typename Targ1, typename Targ2, typename Targ3> class CAmFourArgDelegate: public CAmDelegate
    {
    private:
        TClass* mInstance;
        void (TClass::*mFunction)(Targ argument, Targ1 argument1, Targ2 argument2, Targ3 argument3);
        Targ mArgument;
        Targ1 mArgument1;
        Targ2 mArgument2;
        Targ3 mArgument3;

    public:
        CAmFourArgDelegate(TClass* instance, void (TClass::*function)(Targ argument, Targ1 argument1, Targ2 argument2, Targ3 argument3), Targ argument, Targ1 argument1, Targ2 argument2, Targ3 argument3) :
                mInstance(instance), //
                mFunction(function), //
                mArgument(argument), //
                mArgument1(argument1), //
                mArgument2(argument2), //
                mArgument3(argument3)
        {
        }
        ;

        bool call(int* pipe)
        {
            (void) pipe;
            (*mInstance.*mFunction)(mArgument, mArgument1, mArgument2, mArgument3);
            return (true);
        }
        ;
    };

    /**
     * Template for synchronous calls with no argument
     */
    template<class TClass, typename TretVal> class CAmSyncNoArgDelegate: public CAmDelegate
    {
    private:
        TClass* mInstance;
        TretVal (TClass::*mFunction)();
        TretVal mRetval;

    public:
        friend class CAmSerializer;
        CAmSyncNoArgDelegate(TClass* instance, TretVal (TClass::*function)()) :
                mInstance(instance), //
                mFunction(function), //
                mRetval()
        {
        }
        ;

        bool call(int* pipe)
        {
            mRetval = (*mInstance.*mFunction)();
            write(pipe[1], this, sizeof(this));
            return (false);
        }
        ;

        TretVal returnResults()
        {
            return (mRetval);
        }
    };

    /**
     * template for synchronous calls with one argument
     */
    template<class TClass, typename TretVal, typename TargCall, typename Targ> class CAmSyncOneArgDelegate: public CAmDelegate
    {
    private:
        TClass* mInstance;
        TretVal (TClass::*mFunction)(TargCall argument);
        Targ mArgument;
        TretVal mRetval;

    public:
        friend class CAmSerializer;
        CAmSyncOneArgDelegate(TClass* instance, TretVal (TClass::*function)(TargCall argument), Targ argument) :
                mInstance(instance), //
                mFunction(function), //
                mArgument(argument), //
                mRetval()
        {
        }
        ;

        bool call(int* pipe)
        {
            mRetval = (*mInstance.*mFunction)(mArgument);
            write(pipe[1], this, sizeof(this));
            return (false);
        }
        ;

        TretVal returnResults(Targ& argument)
        {
            argument = mArgument;
            return (mRetval);
        }
    };

    /**
     * template for synchronous calls with one argument on a const function
     */
    template<class TClass, typename TretVal, typename TargCall, typename Targ> class CAmSyncOneArgConstDelegate: public CAmDelegate
    {
    private:
        TClass* mInstance;
        TretVal (TClass::*mFunction)(TargCall argument) const;
        Targ mArgument;
        TretVal mRetval;

    public:
        friend class CAmSerializer;
        CAmSyncOneArgConstDelegate(TClass* instance, TretVal (TClass::*function)(TargCall argument) const, Targ argument) :
                mInstance(instance), //
                mFunction(function), //
                mArgument(argument), //
                mRetval()
        {
        }
        ;

        bool call(int* pipe)
        {
            mRetval = (*mInstance.*mFunction)(mArgument);
            write(pipe[1], this, sizeof(this));
            return (false);
        }
        ;

        TretVal returnResults(Targ& argument)
        {
            argument = mArgument;
            return (mRetval);
        }
    };

    /**
     * template for synchronous calls with two arguments
     */
    template<class TClass, typename TretVal, typename TargCall, typename TargCall1, typename Targ, typename Targ1> class CAmSyncTwoArgDelegate: public CAmDelegate
    {
    private:
        TClass* mInstance;
        TretVal (TClass::*mFunction)(TargCall, TargCall1);
        Targ mArgument;
        Targ1 mArgument1;
        TretVal mRetval;

    public:
        friend class CAmSerializer;
        CAmSyncTwoArgDelegate(TClass* instance, TretVal (TClass::*function)(TargCall, TargCall1), Targ& argument, Targ1& argument1) :
                mInstance(instance), //
                mFunction(function), //
                mArgument(argument), //
                mArgument1(argument1), //
                mRetval()
        {
        }
        ;

        bool call(int* pipe)
        {
            mRetval = (*mInstance.*mFunction)(mArgument, mArgument1);
            write(pipe[1], this, sizeof(this));
            return (false);
        }
        ;

        TretVal returnResults(Targ& argument, Targ1& argument1)
        {
            argument = mArgument;
            argument1 = mArgument1;
            return (mRetval);
        }
    };

    /**
     * template for synchronous calls with two arguments on a const function
     */
    template<class TClass, typename TretVal, typename TargCall, typename TargCall1, typename Targ, typename Targ1> class CAmSyncTwoArgConstDelegate: public CAmDelegate
    {
    private:
        TClass* mInstance;
        TretVal (TClass::*mFunction)(TargCall, TargCall1) const;
        Targ mArgument;
        Targ1 mArgument1;
        TretVal mRetval;

    public:
        friend class CAmSerializer;
        CAmSyncTwoArgConstDelegate(TClass* instance, TretVal (TClass::*function)(TargCall, TargCall1) const, Targ& argument, Targ1& argument1) :
                mInstance(instance), //
                mFunction(function), //
                mArgument(argument), //
                mArgument1(argument1), //
                mRetval()
        {
        }
        ;

        bool call(int* pipe)
        {
            mRetval = (*mInstance.*mFunction)(mArgument, mArgument1);
            write(pipe[1], this, sizeof(this));
            return (false);
        }
        ;

        TretVal returnResults(Targ& argument, Targ1& argument1)
        {
            argument = mArgument;
            argument1 = mArgument1;
            return (mRetval);
        }
    };

    /**
     * template for synchronous calls with three arguments
     */
    template<class TClass, typename TretVal, typename TargCall, typename TargCall1, typename TargCall2, typename Targ, typename Targ1, typename Targ2> class CAmSyncThreeArgDelegate: public CAmDelegate
    {
    private:
        TClass* mInstance;
        TretVal (TClass::*mFunction)(TargCall argument, TargCall1 argument1, TargCall2 argument2);
        Targ mArgument;
        Targ1 mArgument1;
        Targ2 mArgument2;
        TretVal mRetval;

    public:
        CAmSyncThreeArgDelegate(TClass* instance, TretVal (TClass::*function)(TargCall argument, TargCall1 argument1, TargCall2 argument2), Targ argument, Targ1 argument1, Targ2 argument2) :
                mInstance(instance), //
                mFunction(function), //
                mArgument(argument), //
                mArgument1(argument1), //
                mArgument2(argument2), //
                mRetval()
        {
        }
        ;

        bool call(int* pipe)
        {
            mRetval = (*mInstance.*mFunction)(mArgument, mArgument1, mArgument2);
            write(pipe[1], this, sizeof(this));
            return (false);
        }
        ;

        TretVal returnResults(Targ& argument, Targ1& argument1, Targ2& argument2)
        {
            argument = mArgument;
            argument1 = mArgument1;
            argument2 = mArgument2;
            return (mRetval);
        }
    };

    /**
     * template for synchronous calls with four arguments
     */
    template<class TClass, typename TretVal, typename TargCAll, typename TargCall1, typename TargCall2, typename TargCall3, typename Targ, typename Targ1, typename Targ2, typename Targ3> class CAmSyncFourArgDelegate: public CAmDelegate
    {
    private:
        TClass* mInstance;
        TretVal (TClass::*mFunction)(TargCAll argument, TargCall1 argument1, TargCall2 argument2, TargCall3 argument3);
        Targ mArgument;
        Targ1 mArgument1;
        Targ2 mArgument2;
        Targ3 mArgument3;
        TretVal mRetval;
    public:
        CAmSyncFourArgDelegate(TClass* instance, TretVal (TClass::*function)(TargCAll argument, TargCall1 argument1, TargCall2 argument2, TargCall3 argument3), Targ argument, Targ1 argument1, Targ2 argument2, Targ3 argument3) :
                mInstance(instance), //
                mFunction(function), //
                mArgument(argument), //
                mArgument1(argument1), //
                mArgument2(argument2), //
                mArgument3(argument3), //
                mRetval()
        {
        }
        ;

        bool call(int* pipe)
        {
            mRetval = (*mInstance.*mFunction)(mArgument, mArgument1, mArgument2, mArgument3);
            write(pipe[1], this, sizeof(this));
            return (false);
        }
        ;

        TretVal returnResults(Targ& argument, Targ1& argument1, Targ2& argument2, Targ3& argument3)
        {
            argument = mArgument;
            argument1 = mArgument1;
            argument2 = mArgument2;
            argument3 = mArgument3;
            return (mRetval);
        }
    };

    /**
     * delegate template for five arguments
     */
    template<class TClass, typename TretVal, typename TargCAll, typename TargCall1, typename TargCall2, typename TargCall3, typename TargCall4, typename Targ, typename Targ1, typename Targ2, typename Targ3, typename Targ4> class CAmSyncFiveArgDelegate: public CAmDelegate
    {
    private:
        TClass* mInstance;
        TretVal (TClass::*mFunction)(TargCAll argument, TargCall1 argument1, TargCall2 argument2, TargCall3 argument3, TargCall4);
        Targ mArgument;
        Targ1 mArgument1;
        Targ2 mArgument2;
        Targ3 mArgument3;
        Targ4 mArgument4;
        TretVal mRetval;
    public:

        CAmSyncFiveArgDelegate(TClass* instance, TretVal (TClass::*function)(TargCAll argument, TargCall1 argument1, TargCall2 argument2, TargCall3 argument3, TargCall4 argument4), Targ argument, Targ1 argument1, Targ2 argument2, Targ3 argument3, Targ4 argument4) :
                mInstance(instance), //
                mFunction(function), //
                mArgument(argument), //
                mArgument1(argument1), //
                mArgument2(argument2), //
                mArgument3(argument3), //
                mArgument4(argument4), //
                mRetval()
        {
        }
        ;

        bool call(int* pipe)
        {
            mRetval = (*mInstance.*mFunction)(mArgument, mArgument1, mArgument2, mArgument3, mArgument4);
            write(pipe[1], this, sizeof(this));
            return (false);
        }
        ;

        TretVal returnResults(Targ& argument, Targ1& argument1, Targ2& argument2, Targ3& argument3, Targ4& argument4)
        {
            argument = mArgument;
            argument1 = mArgument1;
            argument2 = mArgument2;
            argument3 = mArgument3;
            argument4 = mArgument4;
            return (mRetval);
        }
    };

    /**
     * template for synchronous calls with six arguments
     */
    template<class TClass, typename TretVal, typename TargCAll, typename TargCall1, typename TargCall2, typename TargCall3, typename TargCall4, typename TargCall5, typename Targ, typename Targ1, typename Targ2, typename Targ3, typename Targ4, typename Targ5> class CAmSyncSixArgDelegate: public CAmDelegate
    {
    private:
        TClass* mInstance;
        TretVal (TClass::*mFunction)(TargCAll argument, TargCall1 argument1, TargCall2 argument2, TargCall3 argument3, TargCall4, TargCall5);
        Targ mArgument;
        Targ1 mArgument1;
        Targ2 mArgument2;
        Targ3 mArgument3;
        Targ4 mArgument4;
        Targ5 mArgument5;
        TretVal mRetval;

        CAmSyncSixArgDelegate(TClass* instance, TretVal (TClass::*function)(TargCAll, TargCall1, TargCall2, TargCall3, TargCall4, TargCall5), Targ argument, Targ1 argument1, Targ2 argument2, Targ3 argument3, Targ4 argument4, Targ5 argument5) :
                mInstance(instance), //
                mFunction(function), //
                mArgument(argument), //
                mArgument1(argument1), //
                mArgument2(argument2), //
                mArgument3(argument3), //
                mArgument4(argument4), //
                mArgument5(argument5), //
                mRetval()
        {
        }
        ;

        bool call(int* pipe)
        {
            mRetval = (*mInstance.*mFunction)(mArgument, mArgument1, mArgument2, mArgument3, mArgument4, mArgument5);
            write(pipe[1], this, sizeof(this));
            return (false);
        }
        ;

        TretVal returnResults(Targ& argument, Targ1& argument1, Targ2& argument2, Targ3& argument3, Targ4& argument4, Targ5& argument5)
        {
            argument = mArgument;
            argument1 = mArgument1;
            argument2 = mArgument2;
            argument3 = mArgument3;
            argument4 = mArgument4;
            argument5 = mArgument5;
            return (mRetval);
        }
    };

    /**
     * rings the line of the pipe and adds the delegate pointer to the queue
     * @param p delegate pointer
     */
    inline void send(CAmDelegagePtr p)
    {
        if (write(mPipe[1], &p, sizeof(p)) == -1)
        {
            throw std::runtime_error("could not write to pipe !");
        }
    }

    int mPipe[2]; //!< the pipe
    int mReturnPipe[2]; //!< pipe handling returns
    std::deque<CAmDelegagePtr> mListDelegatePoiters; //!< intermediate queue to store the pipe results

public:

    /**
     * calls a function with no arguments threadsafe
     * @param instance the instance of the class that shall be called
     * @param function the function that shall be called as memberfunction pointer.
     * @tparam TClass1 the type of the Class to be called
     * \section ex Example:
     * @code
     * class myClass
     * {
     * public:
     *      void myfunction();
     * }
     * CAmSerializer serial(&Sockethandler);
     * myClass instanceMyClass;
     * serial<CommandSender>(&instanceMyClass,&myClass::myfunction);
     * @endcode
     */
    template<class TClass>
    void asyncCall(TClass* instance, void (TClass::*function)())
    {
        CAmDelegagePtr p(new CAmNoArgDelegate<TClass>(instance, function));
        send(p);
    }

    /**
     * calls a function with one arguments asynchronously threadsafe
     * @param instance the instance of the class that shall be called
     * @param function the function that shall be called as memberfunction pointer.
     * @param argument the argument
     * @tparam TClass1 the type of the Class to be called
     * @tparam Targ the type of the argument to be called
     * \section ex Example:
     * @code
     * class myClass
     * {
     * public:
     *      void myfunction(int k);
     * }
     * CAmSerializer serial(&Sockethandler);
     * myClass instanceMyClass;
     * serial<CommandSender,int>(&instanceMyClass,&myClass::myfunction,k);
     * @endcode
     *
     */
    template<class TClass1, class Targ>
    void asyncCall(TClass1* instance, void (TClass1::*function)(Targ), Targ argument)
    {
        CAmDelegagePtr p(new CAmOneArgDelegate<TClass1, Targ>(instance, function, argument));
        send(p);
    }

    /**
     * calls a function with one argument called by reference asynchronously threadsafe
     * @param instance the instance of the class that shall be called
     * @param function the function that shall be called as memberfunction pointer.
     * @param argument the argument
     * @tparam TClass1 the type of the Class to be called
     * @tparam Targ the type of the argument to be called
     * \section ex Example:
     * @code
     * class myClass
     * {
     * public:
     *      void myfunction(int k);
     * }
     * CAmSerializer serial(&Sockethandler);
     * myClass instanceMyClass;
     * serial<CommandSender,int>(&instanceMyClass,&myClass::myfunction,k);
     * @endcode
     *
     */
    template<class TClass1, class Targ>
    void asyncCall(TClass1* instance, void (TClass1::*function)(Targ&), Targ& argument)
    {
        CAmDelegagePtr p(new CAmOneArgDelegate<TClass1, Targ&>(instance, function, argument));
        send(p);
    }

    /**
     * calls a function with two arguments asynchronously threadsafe. for more see asyncCall with one argument
     * @param instance pointer to the instance of the class
     * @param function memberfunction poitner
     * @param argument the first argument
     * @param argument1 the second argument
     * @tparam TClass1 the type of the Class to be called
     * @tparam Targ the type of the argument to be called
     * @tparam Targ1 the type of the first argument to be called
     */
    template<class TClass1, class Targ, class Targ1>
    void asyncCall(TClass1* instance, void (TClass1::*function)(Targ argument, Targ1 argument1), Targ argument, Targ1 argument1)
    {
        logInfo("took without ref");
        CAmDelegagePtr p(new CAmTwoArgDelegate<TClass1, Targ, Targ1>(instance, function, argument, argument1));
        send(p);
    }

    /**
     * calls a function with two arguments asynchronously threadsafe, first argument is a reference. for more see asyncCall with one argument
     * @param instance pointer to the instance of the class
     * @param function memberfunction poitner
     * @param argument the first argument
     * @param argument1 the second argument
     * @tparam TClass1 the type of the Class to be called
     * @tparam Targ the type of the argument to be called
     * @tparam Targ1 the type of the first argument to be called
     */
    template<class TClass1, class Targ, class Targ1>
    void asyncCall(TClass1* instance, void (TClass1::*function)(Targ& argument, Targ1 argument1), Targ& argument, Targ1 argument1)
    {
        CAmDelegagePtr p(new CAmTwoArgDelegateFirstRef<TClass1, Targ, Targ1>(instance, function, argument, argument1));
        send(p);
    }

    /**
     * calls a function with two arguments asynchronously threadsafe, second argument is a reference. for more see asyncCall with one argument
     * @param instance pointer to the instance of the class
     * @param function memberfunction poitner
     * @param argument the first argument
     * @param argument1 the second argument
     * @tparam TClass1 the type of the Class to be called
     * @tparam Targ the type of the argument to be called
     * @tparam Targ1 the type of the first argument to be called
     */
    template<class TClass1, class Targ, class Targ1>
    void asyncCall(TClass1* instance, void (TClass1::*function)(Targ argument, Targ1& argument1), Targ argument, Targ1& argument1)
    {
        logInfo("took ref");
        CAmDelegagePtr p(new CAmTwoArgDelegateSecondRef<TClass1, Targ, Targ1>(instance, function, argument, argument1));
        send(p);
    }

    /**
     * calls a function with two arguments asynchronously threadsafe, both arguments are references. for more see asyncCall with one argument
     * @param instance pointer to the instance of the class
     * @param function memberfunction poitner
     * @param argument the first argument
     * @param argument1 the second argument
     * @tparam TClass1 the type of the Class to be called
     * @tparam Targ the type of the argument to be called
     * @tparam Targ1 the type of the first argument to be called
     */
    template<class TClass1, class Targ, class Targ1>
    void asyncCall(TClass1* instance, void (TClass1::*function)(Targ& argument, Targ1& argument1), Targ& argument, Targ1& argument1)
    {
        CAmDelegagePtr p(new CAmTwoArgDelegateAllRef<TClass1, Targ, Targ1>(instance, function, argument, argument1));
        send(p);
    }

    /**
     * calls a function with three arguments asynchronously threadsafe. for more see other asycCall
     */
    template<class TClass1, class Targ, class Targ1, class Targ2>
    void asyncCall(TClass1* instance, void (TClass1::*function)(Targ argument, Targ1 argument1, Targ2 argument2), Targ argument, Targ1 argument1, Targ2 argument2)
    {
        CAmDelegagePtr p(new CAmThreeArgDelegate<TClass1, Targ, Targ1, Targ2>(instance, function, argument, argument1, argument2));
        send(p);
    }

    /**
     * calls a function with three arguments asynchronously threadsafe. for more see other asycCall
     */
    template<class TClass1, class Targ, class Targ1, class Targ2>
    void asyncCall(TClass1* instance, void (TClass1::*function)(Targ& argument, Targ1 argument1, Targ2 argument2), Targ& argument, Targ1 argument1, Targ2 argument2)
    {
        CAmDelegagePtr p(new CAmThreeArgDelegateFirstRef<TClass1, Targ, Targ1, Targ2>(instance, function, argument, argument1, argument2));
        send(p);
    }


    /**
     * calls a function with three arguments asynchronously threadsafe. for more see other asycCall
     */
    template<class TClass1, class Targ, class Targ1, class Targ2>
    void asyncCall(TClass1* instance, void (TClass1::*function)(Targ argument, Targ1& argument1, Targ2 argument2), Targ argument, Targ1& argument1, Targ2 argument2)
    {
        CAmDelegagePtr p(new CAmThreeArgDelegateSecondRef<TClass1, Targ, Targ1, Targ2>(instance, function, argument, argument1, argument2));
        send(p);
    }

    /**
     * calls a function with three arguments asynchronously threadsafe. for more see other asycCall
     */
    template<class TClass1, class Targ, class Targ1, class Targ2>
    void asyncCall(TClass1* instance, void (TClass1::*function)(Targ argument, Targ1 argument1, Targ2& argument2), Targ argument, Targ1 argument1, Targ2& argument2)
    {
        CAmDelegagePtr p(new CAmThreeArgDelegateThirdRef<TClass1, Targ, Targ1, Targ2>(instance, function, argument, argument1, argument2));
        send(p);
    }

    /**
     * calls a function with three arguments asynchronously threadsafe. for more see other asycCall
     */
    template<class TClass1, class Targ, class Targ1, class Targ2>
    void asyncCall(TClass1* instance, void (TClass1::*function)(Targ argument, Targ1& argument1, Targ2& argument2), Targ argument, Targ1& argument1, Targ2& argument2)
    {
        CAmDelegagePtr p(new CAmThreeArgDelegateSecondThirdRef<TClass1, Targ, Targ1, Targ2>(instance, function, argument, argument1, argument2));
        send(p);
    }

    /**
     * calls a function with three arguments asynchronously threadsafe. for more see other asycCall
     */
    template<class TClass1, class Targ, class Targ1, class Targ2>
    void asyncCall(TClass1* instance, void (TClass1::*function)(Targ& argument, Targ1& argument1, Targ2& argument2), Targ& argument, Targ1& argument1, Targ2& argument2)
    {
        CAmDelegagePtr p(new CAmThreeArgDelegateAllRef<TClass1, Targ, Targ1, Targ2>(instance, function, argument, argument1, argument2));
        send(p);
    }

    /**
     * calls a function with three arguments asynchronously threadsafe. for more see other asycCall
     */
    template<class TClass1, class Targ, class Targ1, class Targ2>
    void asyncCall(TClass1* instance, void (TClass1::*function)(Targ& argument, Targ1& argument1, Targ2 argument2), Targ& argument, Targ1& argument1, Targ2 argument2)
    {
        CAmDelegagePtr p(new CAmThreeArgDelegateFirstSecondRef<TClass1, Targ, Targ1, Targ2>(instance, function, argument, argument1, argument2));
        send(p);
    }

    /**
     * calls a function with three arguments asynchronously threadsafe. for more see other asycCall
     */
    template<class TClass1, class Targ, class Targ1, class Targ2>
    void asyncCall(TClass1* instance, void (TClass1::*function)(Targ& argument, Targ1 argument1, Targ2& argument2), Targ& argument, Targ1 argument1, Targ2& argument2)
    {
        CAmDelegagePtr p(new CAmThreeArgDelegateFirstThirdRef<TClass1, Targ, Targ1, Targ2>(instance, function, argument, argument1, argument2));
        send(p);
    }

    /**
     * calls a function with four arguments asynchronously threadsafe. for more see other asycCall
     */
    template<class TClass1, class Targ, class Targ1, class Targ2, class Targ3>
    void asyncCall(TClass1* instance, void (TClass1::*function)(Targ argument, Targ1 argument1, Targ2 argument2, Targ3 argument3), Targ argument, Targ1 argument1, Targ2 argument2, Targ3 argument3)
    {
        CAmDelegagePtr p(new CAmFourArgDelegate<TClass1, Targ, Targ1, Targ2, Targ3>(instance, function, argument, argument1, argument2, argument3));
        send(p);
    }

    /**
     * calls a synchronous function with no arguments threadsafe
     * @param instance the instance of the class that shall be called
     * @param function the function that shall be called as memberfunction pointer.
     * @param retVal the return parameter, no const allowed !
     * @tparam TClass1 the type of the class to be called
     * @tparam TretVal the type of the return parameter
     * \section ex Example:
     * @code
     * class myClass
     * {
     * public:
     *      am_Error_e myfunction();
     * }
     * CAmSerializer serial(&Sockethandler);
     * myClass instanceMyClass;
     * am_Error_e error;
     * serial<CommandSender,am_Error_e>(&instanceMyClass,&myClass::myfunction, error);
     * @endcode
     * All arguments given to synchronous functions must be non-const since the results of the operations will be written back to the arguments.
     *
     */
    template<class TClass1, class TretVal>
    void syncCall(TClass1* instance, TretVal (TClass1::*function)(), TretVal& retVal)
    {
        CAmSyncNoArgDelegate<TClass1, TretVal>* p(new CAmSyncNoArgDelegate<TClass1, TretVal>(instance, function));
        send(static_cast<CAmDelegagePtr>(p));
        int numReads;
        CAmDelegagePtr ptr;
        if ((numReads = read(mReturnPipe[0], &ptr, sizeof(ptr))) == -1)
        {
            logError("CAmSerializer::receiverCallback could not read pipe!");
            throw std::runtime_error("CAmSerializer Could not read pipe!");
        }
        //working with friend class here is not the finest of all programming stiles but it works...
        retVal = p->returnResults();
        delete p;
    }

    /**
     * calls a function with one argument synchronous threadsafe
     * @param instance the instance of the class that shall be called
     * @param function the function that shall be called as memberfunction pointer.
     * @param retVal the return parameter, no const allowed !
     * @param argument the argument, no const allowed !
     * @tparam TClass1 the type of the class to be called
     * @tparam TretVal the type of the return parameter
     * @tparam TargCall the type of the argument like in the function to be called. here all references and const must be
     * respected!
     * @tparam Targ the type of the argument, here no const and no references allowed !
     * \section ex Example:
     * @code
     * class myClass
     * {
     * public:
     *      am_Error_e myfunction(int k);
     * }
     * CAmSerializer serial(&Sockethandler);
     * myClass instanceMyClass;
     * am_Error_e error;
     * int l;
     * serial<CommandSender,am_Error_e,int>(&instanceMyClass,&myClass::myfunction,error,l);
     * @endcode
     * All arguments given to synchronous functions must be non-const since the results of the operations will be written back to the arguments.
     */
    template<class TClass1, class TretVal, class TargCall, class Targ>
    void syncCall(TClass1* instance, TretVal (TClass1::*function)(TargCall), TretVal& retVal, Targ& argument)
    {
        CAmSyncOneArgDelegate<TClass1, TretVal, TargCall, Targ>* p(new CAmSyncOneArgDelegate<TClass1, TretVal, TargCall, Targ>(instance, function, argument));
        send(static_cast<CAmDelegagePtr>(p));
        int numReads;
        CAmDelegagePtr ptr;
        if ((numReads = read(mReturnPipe[0], &ptr, sizeof(ptr))) == -1)
        {
            logError("CAmSerializer::receiverCallback could not read pipe!");
            throw std::runtime_error("CAmSerializer Could not read pipe!");
        }
        //working with friend class here is not the finest of all programming stiles but it works...
        retVal = p->returnResults(argument);
        delete p;
    }

    /**
     * calls a function with one argument synchronous threadsafe for const functions. For more see syncCall with one argument
     */
    template<class TClass1, class TretVal, class TargCall, class Targ>
    void syncCall(TClass1* instance, TretVal (TClass1::*function)(TargCall) const, TretVal& retVal, Targ& argument)
    {
        CAmSyncOneArgConstDelegate<TClass1, TretVal, TargCall, Targ>* p(new CAmSyncOneArgConstDelegate<TClass1, TretVal, TargCall, Targ>(instance, function, argument));
        send(static_cast<CAmDelegagePtr>(p));
        int numReads;
        CAmDelegagePtr ptr;
        if ((numReads = read(mReturnPipe[0], &ptr, sizeof(ptr))) == -1)
        {
            logError("CAmSerializer::receiverCallback could not read pipe!");
            throw std::runtime_error("CAmSerializer Could not read pipe!");
        }
        //working with friend class here is not the finest of all programming stiles but it works...
        retVal = p->returnResults(argument);
        delete p;
    }

    /**
     * calls a function with two arguments synchronously threadsafe. For more see syncCall with one argument
     */
    template<class TClass1, class TretVal, class TargCall, class Targ1Call, class Targ, class Targ1>
    void syncCall(TClass1* instance, TretVal (TClass1::*function)(TargCall, Targ1Call), TretVal& retVal, Targ& argument, Targ1& argument1)
    {
        CAmSyncTwoArgDelegate<TClass1, TretVal, TargCall, Targ1Call, Targ, Targ1>* p(new CAmSyncTwoArgDelegate<TClass1, TretVal, TargCall, Targ1Call, Targ, Targ1>(instance, function, argument, argument1));
        send(dynamic_cast<CAmDelegagePtr>(p));

        CAmDelegagePtr ptr;
        if (read(mReturnPipe[0], &ptr, sizeof(ptr)) == -1)
        {
            logError("CAmSerializer::receiverCallback could not read pipe!");
            throw std::runtime_error("CAmSerializer Could not read pipe!");
        }
        retVal = p->returnResults(argument, argument1);
        delete p;
    }
    /**
     * calls a function with two arguments synchronously threadsafe const. For more see syncCall with one argument
     */
    template<class TClass1, class TretVal, class TargCall, class Targ1Call, class Targ, class Targ1>
    void syncCall(TClass1* instance, TretVal (TClass1::*function)(TargCall, Targ1Call) const, TretVal& retVal, Targ& argument, Targ1& argument1)
    {
        CAmSyncTwoArgConstDelegate<TClass1, TretVal, TargCall, Targ1Call, Targ, Targ1>* p(new CAmSyncTwoArgConstDelegate<TClass1, TretVal, TargCall, Targ1Call, Targ, Targ1>(instance, function, argument, argument1));
        send(dynamic_cast<CAmDelegagePtr>(p));

        CAmDelegagePtr ptr;
        if (read(mReturnPipe[0], &ptr, sizeof(ptr)) == -1)
        {
            logError("CAmSerializer::receiverCallback could not read pipe!");
            throw std::runtime_error("CAmSerializer Could not read pipe!");
        }
        retVal = p->returnResults(argument, argument1);
        delete p;
    }

    /**
     * calls a function with three arguments synchronously threadsafe. for more see syncCall with one argument
     */
    template<class TClass1, class TretVal, class TargCall, class TargCall1, class TargCall2, class Targ, class Targ1, class Targ2>
    void syncCall(TClass1* instance, TretVal (TClass1::*function)(TargCall, TargCall1, TargCall2), TretVal& retVal, Targ& argument, Targ1& argument1, Targ2& argument2)
    {
        CAmSyncThreeArgDelegate<TClass1, TretVal, TargCall, TargCall1, TargCall2, Targ, Targ1, Targ2>* p(new CAmSyncThreeArgDelegate<TClass1, TretVal, TargCall, TargCall1, TargCall2, Targ, Targ1, Targ2>(instance, function, argument, argument1, argument2));
        send(static_cast<CAmDelegagePtr>(p));
        int numReads;
        CAmDelegagePtr ptr;
        if ((numReads = read(mReturnPipe[0], &ptr, sizeof(ptr))) == -1)
        {
            logError("CAmSerializer::receiverCallback could not read pipe!");
            throw std::runtime_error("CAmSerializer Could not read pipe!");
        }
        //working with friend class here is not the finest of all programming stiles but it worCAmTwoArgDelegateks...
        retVal = p->returnResults(argument, argument1, argument2);
        delete p;
    }

    /**
     * calls a function with four arguments synchronously threadsafe. for more see syncCall with one argument
     */
    template<class TClass1, class TretVal, class TargCall, class TargCall1, class TargCall2, class TargCall3, class Targ, class Targ1, class Targ2, class Targ3>
    void syncCall(TClass1* instance, TretVal (TClass1::*function)(TargCall, TargCall1, TargCall2, TargCall3), TretVal& retVal, Targ& argument, Targ1& argument1, Targ2& argument2, Targ3& argument3)
    {
        CAmSyncFourArgDelegate<TClass1, TretVal, TargCall, TargCall1, TargCall2, TargCall3, Targ, Targ1, Targ2, Targ3>* p(new CAmSyncFourArgDelegate<TClass1, TretVal, TargCall, TargCall1, TargCall2, TargCall3, Targ, Targ1, Targ2, Targ3>(instance, function, argument, argument1, argument2, argument3));
        send(static_cast<CAmDelegagePtr>(p));
        int numReads;
        CAmDelegagePtr ptr;
        if ((numReads = read(mReturnPipe[0], &ptr, sizeof(ptr))) == -1)
        {
            logError("CAmSerializer::receiverCallback could not read pipe!");
            throw std::runtime_error("CAmSerializer Could not read pipe!");
        }
        //working with friend class here is not the finest of all programming stiles but it works...
        retVal = p->returnResults(argument, argument1, argument2, argument3);
        delete p;
    }

    /**
     * calls a function with five arguments synchronously threadsafe. for more see syncCall with one argument
     */
    template<class TClass1, class TretVal, class TargCall, class TargCall1, class TargCall2, class TargCall3, class TargCall4, class Targ, class Targ1, class Targ2, class Targ3, class Targ4>
    void syncCall(TClass1* instance, TretVal (TClass1::*function)(TargCall, TargCall1, TargCall2, TargCall3, TargCall4), TretVal& retVal, Targ& argument, Targ1& argument1, Targ2& argument2, Targ3& argument3, Targ4& argument4)
    {
        CAmSyncFiveArgDelegate<TClass1, TretVal, TargCall, TargCall1, TargCall2, TargCall3, TargCall4, Targ, Targ1, Targ2, Targ3, Targ4>* p(new CAmSyncFiveArgDelegate<TClass1, TretVal, TargCall, TargCall1, TargCall2, TargCall3, TargCall4, Targ, Targ1, Targ2, Targ3, Targ4>(instance, function, argument, argument1, argument2, argument3, argument4));
        send(static_cast<CAmDelegagePtr>(p));
        int numReads;
        CAmDelegagePtr ptr;
        if ((numReads = read(mReturnPipe[0], &ptr, sizeof(ptr))) == -1)
        {
            logError("CAmSerializer::receiverCallback could not read pipe!");
            throw std::runtime_error("CAmSerializer Could not read pipe!");
        }
        //working with friend class here is not the finest of all programming stiles but it works...
        retVal = p->returnResults(argument, argument1, argument2, argument3, argument4);
        delete p;
    }

    /**
     * calls a function with six arguments synchronously threadsafe. for more see syncCall with one argument
     */
    template<class TClass1, class TretVal, class TargCall, class TargCall1, class TargCall2, class TargCall3, class TargCall4, class TargCall5, class Targ, class Targ1, class Targ2, class Targ3, class Targ4, class Targ5>
    void syncCall(TClass1* instance, TretVal (TClass1::*function)(TargCall, TargCall1, TargCall2, TargCall3, TargCall4, TargCall5), TretVal& retVal, Targ& argument, Targ1& argument1, Targ2& argument2, Targ3& argument3, Targ4& argument4, Targ5& argument5)
    {
        CAmSyncSixArgDelegate<TClass1, TretVal, TargCall, TargCall1, TargCall2, TargCall3, TargCall4, TargCall5, Targ, Targ1, Targ2, Targ3, Targ4, Targ5>* p(new CAmSyncSixArgDelegate<TClass1, TretVal, TargCall, TargCall1, TargCall2, TargCall3, TargCall4, TargCall5, Targ, Targ1, Targ2, Targ3, Targ4, Targ5>(instance, function, argument, argument1, argument2, argument3, argument4, argument5));
        send(static_cast<CAmDelegagePtr>(p));
        int numReads;
        CAmDelegagePtr ptr;
        if ((numReads = read(mReturnPipe[0], &ptr, sizeof(ptr))) == -1)
        {
            logError("CAmSerializer::receiverCallback could not read pipe!");
            throw std::runtime_error("CAmSerializer Could not read pipe!");
        }
        //working with friend class here is not the finest of all programming stiles but it works...
        retVal = p->returnResults(argument, argument1, argument2, argument3, argument4, argument5);
        delete p;
    }

    /**
     * receiver callback for sockethandling, for more, see CAmSocketHandler
     */
    void receiverCallback(const pollfd pollfd, const sh_pollHandle_t handle, void* userData)
    {
        (void) handle;
        (void) userData;
        int numReads;
        CAmDelegagePtr listPointers[3];
        if ((numReads = read(pollfd.fd, &listPointers, sizeof(listPointers))) == -1)
        {
            logError("CAmSerializer::receiverCallback could not read pipe!");
            throw std::runtime_error("CAmSerializer Could not read pipe!");
        }
        mListDelegatePoiters.assign(listPointers, listPointers + (numReads / sizeof(CAmDelegagePtr)));
    }

    /**
     * checker callback for sockethandling, for more, see CAmSocketHandler
     */
    bool checkerCallback(const sh_pollHandle_t handle, void* userData)
    {
        (void) handle;
        (void) userData;
        if (mListDelegatePoiters.empty())
            return (false);
        return (true);
    }

    /**
     * dispatcher callback for sockethandling, for more, see CAmSocketHandler
     */
    bool dispatcherCallback(const sh_pollHandle_t handle, void* userData)
    {
        (void) handle;
        (void) userData;
        CAmDelegagePtr delegatePoiter = mListDelegatePoiters.front();
        mListDelegatePoiters.pop_front();
        if (delegatePoiter->call(mReturnPipe))
            delete delegatePoiter;
        if (mListDelegatePoiters.empty())
            return (false);
        return (true);
    }

    TAmShPollFired<CAmSerializer> receiverCallbackT;
    TAmShPollDispatch<CAmSerializer> dispatcherCallbackT;
    TAmShPollCheck<CAmSerializer> checkerCallbackT;

    /**
     * The constructor must be called in the mainthread context !
     * @param iSocketHandler pointer to the CAmSocketHandler
     */
    CAmSerializer(CAmSocketHandler *iSocketHandler) :
            mPipe(), //
            mListDelegatePoiters(), //
            receiverCallbackT(this, &CAmSerializer::receiverCallback), //
            dispatcherCallbackT(this, &CAmSerializer::dispatcherCallback), //
            checkerCallbackT(this, &CAmSerializer::checkerCallback)
    {
        if (pipe(mPipe) == -1)
        {
            logError("CAmSerializer could not create pipe!");
            throw std::runtime_error("CAmSerializer Could not open pipe!");
        }

        if (pipe(mReturnPipe) == -1)
        {
            logError("CAmSerializer could not create mReturnPipe!");
            throw std::runtime_error("CAmSerializer Could not open mReturnPipe!");
        }

        short event = 0;
        sh_pollHandle_t handle;
        event |= POLLIN;
        iSocketHandler->addFDPoll(mPipe[0], event, NULL, &receiverCallbackT, &checkerCallbackT, &dispatcherCallbackT, NULL, handle);
    }

    ~CAmSerializer()
    {
    }
};
} /* namespace am */
#endif /* CAMSERIALIZER_H_ */
