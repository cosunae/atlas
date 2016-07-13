/*
 * (C) Copyright 1996-2016 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include <sstream>
#include <map>
#include <string>
#include "eckit/exception/Exceptions.h"
#include "eckit/os/BackTrace.h"
#include "eckit/thread/AutoLock.h"
#include "eckit/thread/Mutex.h"
#include "atlas/grid/Grid.h"
#include "atlas/field/FieldCreator.h"
#include "atlas/field/FieldCreatorArraySpec.h"
#include "atlas/field/FieldCreatorIFS.h"
#include "atlas/field/Field.h"
#include "atlas/runtime/Log.h"

namespace {
    static eckit::Mutex *local_mutex = 0;
    static std::map<std::string, atlas::field::FieldCreatorFactory *> *m = 0;
    static pthread_once_t once = PTHREAD_ONCE_INIT;

    static void init() {
        local_mutex = new eckit::Mutex();
        m = new std::map<std::string, atlas::field::FieldCreatorFactory *>();
    }
}


namespace atlas {
namespace field {

namespace {

template<typename T> void load_builder(const std::string& name) { FieldCreatorBuilder<T> tmp(name); }
struct force_link {
    force_link()
    {
        load_builder< FieldCreatorIFS >("tmp_IFS");
        load_builder< FieldCreatorArraySpec >("tmp_ArraySpec");
    }
};

}





// ------------------------------------------------------------------

FieldCreator::FieldCreator()
{
}

FieldCreator::~FieldCreator()
{
}

FieldCreatorFactory::FieldCreatorFactory(const std::string &name):
    name_(name) {

    pthread_once(&once, init);

    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    if( m->find(name) != m->end() )
    {
      std::string backtrace = eckit::BackTrace::dump();
      throw eckit::SeriousBug("FieldCreatorFactory ["+name+"] already registered\n\nBacktrace:\n"+backtrace,Here());
    }
    (*m)[name] = this;
}


FieldCreatorFactory::~FieldCreatorFactory() {
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);
    m->erase(name_);
}


void FieldCreatorFactory::list(std::ostream& out) {
    pthread_once(&once, init);

    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    static force_link static_linking;

    const char* sep = "";
    for (std::map<std::string, FieldCreatorFactory *>::const_iterator j = m->begin() ; j != m->end() ; ++j) {
        out << sep << (*j).first;
        sep = ", ";
    }
}


FieldCreator *FieldCreatorFactory::build(const std::string &name) {

    pthread_once(&once, init);

    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    static force_link static_linking;

    std::map<std::string, FieldCreatorFactory *>::const_iterator j = m->find(name);

    Log::debug() << "Looking for FieldCreatorFactory [" << name << "]" << '\n';

    if (j == m->end()) {
        Log::error() << "No FieldCreatorFactory for [" << name << "]" << '\n';
        Log::error() << "FieldCreatorFactories are:" << '\n';
        for (j = m->begin() ; j != m->end() ; ++j)
            Log::error() << "   " << (*j).first << '\n';
        throw eckit::SeriousBug(std::string("No FieldCreatorFactory called ") + name);
    }

    return (*j).second->make();
}


FieldCreator *FieldCreatorFactory::build(const std::string& name, const eckit::Parametrisation& param) {

    pthread_once(&once, init);

    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    static force_link static_linking;

    std::map<std::string, FieldCreatorFactory *>::const_iterator j = m->find(name);

    Log::debug() << "Looking for FieldCreatorFactory [" << name << "]" << '\n';

    if (j == m->end()) {
        Log::error() << "No FieldCreatorFactory for [" << name << "]" << '\n';
        Log::error() << "FieldCreatorFactories are:" << '\n';
        for (j = m->begin() ; j != m->end() ; ++j)
            Log::error() << "   " << (*j).first << '\n';
        throw eckit::SeriousBug(std::string("No FieldCreatorFactory called ") + name);
    }

    return (*j).second->make(param);
}

} // namespace field
} // namespace atlas
