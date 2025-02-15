// Observable.h

#ifndef _UTIL_EVENT_OBSERVABLE_H_
#define _UTIL_EVENT_OBSERVABLE_H_

#include <boost/function.hpp>

namespace util
{
    namespace event
    {

        struct EventId;
        class Event;

        typedef boost::function<void (Event const &)> Listener;

        class Observable
        {
        public:
            void on(
                EventId const & e, 
                Listener const & l);

            template <typename E>
            void on(
                Listener const & l)
            {
                on(E::id, l);
            }

            void un(
                EventId const & e, 
                Listener const & l);

            template <typename E>
            void un(
                Listener const & l)
            {
                un(E::id, l);
            }

        protected:
            void raise(
                Event const & e);

        private:
            std::map<EventId const *, std::list<Listener> > listeners_;
        };

    } // namespace event
} // namespace util

#endif // _UTIL_EVENT_OBSERVABLE_H_
