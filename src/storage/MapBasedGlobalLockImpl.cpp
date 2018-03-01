#include "MapBasedGlobalLockImpl.h"

#include <mutex>

namespace Afina {
namespace Backend {

// See MapBasedGlobalLockImpl.h
bool MapBasedGlobalLockImpl::Put(const std::string &key, const std::string &value) {
        mut.lock();
        if (_backend.size()==_max_size){
            auto first_elem = _backend_list.front();
            _backend.erase(first_elem);
            _backend_list.pop_front();

        }

        if (_backend.count(key)!=0){
            _backend.erase(key);
            _backend_list.remove(key);
        }
        if((_backend.insert( std::pair<std::string,std::string>(key,value)).second)){
            _backend_list.push_back(key);
            mut.unlock();
            return true;
        }
        else{
            mut.unlock();
            return false;
        }
        //return false;
    }

// See MapBasedGlobalLockImpl.h
bool MapBasedGlobalLockImpl::PutIfAbsent(const std::string &key, const std::string &value) {
        mut.lock();
        if (_backend.count(key)!=0){
            mut.unlock();
            return false;
        }

        if (_backend.size()==_max_size){
            auto first_elem = _backend_list.front();
            _backend.erase(first_elem);
            _backend_list.pop_front();

        }

        if((_backend.insert( std::pair<std::string,std::string>(key,value)).second)){
            _backend_list.push_back(key);
            mut.unlock();
            return true;
        }
        else{
            mut.unlock();
            return false;
        }
        //return false;
    }

// See MapBasedGlobalLockImpl.h
bool MapBasedGlobalLockImpl::Set(const std::string &key, const std::string &value) {
        mut.lock();
        if (_backend.count(key)==0){
            mut.unlock();
            return false;
        }
        else{
            _backend.erase(key);
            _backend_list.remove(key);
            if((_backend.insert( std::pair<std::string,std::string>(key,value)).second)){
                mut.unlock();
                return true;
            }
            else{
                mut.unlock();
                return false;
            }
        }

        //return false;
    }

// See MapBasedGlobalLockImpl.h
bool MapBasedGlobalLockImpl::Delete(const std::string &key) { //
        mut.lock();
        if (_backend.count(key)==0){
            mut.unlock();
            return false;
        }
        else{
            _backend.erase(key);
            _backend_list.remove(key);
            if(_backend.count(key)==0){
                mut.unlock();
                return true;
            }
            mut.unlock();
            return false;
        }

        // return false;
    }

// See MapBasedGlobalLockImpl.h
bool MapBasedGlobalLockImpl::Get(const std::string &key, std::string &value) const {
        mut.lock();
        if(_backend.count(key)==0){
            mut.unlock();
            return false;
        }
        auto size = _backend.size();
        auto it = _backend.find(key);
        if (it != _backend.end()){
            value.clear();
            value.append(it->second);
            mut.unlock();
            return true;
        }
        mut.unlock();
        return false;
        //return false;
    }

} // namespace Backend
} // namespace Afina
