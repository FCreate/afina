#include "MapBasedGlobalLockImpl.h"

#include <mutex>

namespace Afina {
namespace Backend {

// See MapBasedGlobalLockImpl.h
bool MapBasedGlobalLockImpl::Put(const std::string &key, const std::string &value) {
        mut.lock();
        size_t size_element = key.size() + value.size();
        if (size_element>_max_size){
            mut.unlock();
            return false;
        }
            auto flag = _backend.find(key);
            if ((flag==_backend.end())){
                while(_now_size+size_element>_max_size){
                    std::string key_for_del = _backend_list.front()->key;
                    _now_size-=(key_for_del.size()+_backend_list.front()->data.size());
                    _backend.erase(key_for_del);
                    _backend_list.remove(_backend_list.front());
                }
                _backend_list.push_back(value);
                _backend_list.end()->key = key;
                _backend[_backend_list.end()->key]=_backend_list.end();
                _now_size+=size_element;
                mut.unlock();
                return true;

            }
            else if (flag!=_backend.end()){
                _now_size =_now_size+value.size() - flag->second->data.size();
                flag->second->data = value;
                _backend_list.set_end(flag->second);
                while(_now_size+value.size() - flag->second->data.size()>_max_size){
                    std::string key_for_del = _backend_list.front()->key;
                    _now_size-=(key_for_del.size()+_backend_list.front()->data.size());
                    _backend.erase(key_for_del);
                    _backend_list.remove(_backend_list.front());
                }

                mut.unlock();
                return true;
            }
            else{
            mut.unlock();
            return false;
        }
    }


// See MapBasedGlobalLockImpl.h
bool MapBasedGlobalLockImpl::PutIfAbsent(const std::string &key, const std::string &value) {
        mut.lock();
        size_t size_element = key.size() + value.size();
        if (size_element>_max_size){
            mut.unlock();
            return false;
        }
        auto flag = _backend.find(key);
        if ((flag==_backend.end())&&(_now_size+size_element<=_max_size)){
            _backend_list.push_back(value);
            _backend_list.end()->key = key;
            _backend[_backend_list.end()->key]=_backend_list.end();
            _now_size+=size_element;
            mut.unlock();
            return true;

        }
        else{
            mut.unlock();
            return false;
        }
        /*
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
        //return false;*/
    }

// See MapBasedGlobalLockImpl.h
bool MapBasedGlobalLockImpl::Set(const std::string &key, const std::string &value) {
        mut.lock();
        size_t size_element = key.size() + value.size();
        auto flag = _backend.find(key);
        if ((size_element>_max_size)||
                ((flag!=_backend.end())&&(_now_size+value.size() - flag->second->data.size()<=_max_size))) {
            mut.unlock();
            return false;
        }

        if ((flag!=_backend.end())&&(_now_size+value.size() - flag->second->data.size()<=_max_size)){
            _now_size =_now_size+value.size() - flag->second->data.size();
            flag->second->data = value;
            _backend_list.set_end(flag->second);
            mut.unlock();
            return true;
        }
        else{
            mut.unlock();
            return false;
        }




        /*if (_backend.count(key)==0){
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

        //return false;*/
    }

// See MapBasedGlobalLockImpl.h
bool MapBasedGlobalLockImpl::Delete(const std::string &key) {

        mut.lock();
        auto flag = _backend.find(key);
        if (flag!=_backend.end()){
            size_t size_element = flag->second->data.size()+flag->second->key.size();
            _now_size -=size_element;
            _backend.erase(key);
            _backend_list.remove(key);
            mut.unlock();
            return true;
        }
        else{
            mut.unlock();
            return false;
        }


/*
        if (_backend.count(key)==0){
            mut.unlock();
            return false;
        }
        else{
            _backend.erase(key);
            _backend_list.remove(key);auto flag = _backend.find(key);find
            if(_backend.count(key)==0){
                mut.unlock();
                return true;
            }
            mut.unlock();
            return false;
        }

        // return false;*/
    }

// See MapBasedGlobalLockImpl.h
bool MapBasedGlobalLockImpl::Get(const std::string &key, std::string &value) const {
        mut.lock();
        auto flag = _backend.find(key);
        if (flag ==_backend.end()){
            mut.unlock();
            return false;
        }
        else{
            value= flag->second->data;
            _backend_list.set_end(flag->second);
            mut.unlock();
            return true;
        }
        /*if(_backend.count(key)==0){
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
        //return false;*/
    }

} // namespace Backend
} // namespace Afina
