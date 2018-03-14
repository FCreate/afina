#include "MapBasedGlobalLockImpl.h"

namespace Afina {
namespace Backend {

// See MapBasedGlobalLockImpl.h
bool MapBasedGlobalLockImpl::Put(const std::string &key, const std::string &value) {
        std::lock_guard<std::mutex> lck(mut);
        size_t size_element = key.size() + value.size();
        if (size_element>_max_size){
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
                return true;

            }
            else if (flag!=_backend.end()){
                while(_now_size+value.size() - flag->second->data.size()>_max_size){
                    if(flag->second!= _backend_list.front()){
                        std::string key_for_del = _backend_list.front()->key;
                        _now_size-=(key_for_del.size()+_backend_list.front()->data.size());
                        _backend.erase(key_for_del);
                        _backend_list.remove(_backend_list.front());
                    }
                    else{
                        std::string key_for_del = _backend_list.front()->next->key;
                        _now_size-=(key_for_del.size()+_backend_list.front()->next->data.size());
                        _backend.erase(key_for_del);
                        _backend_list.remove(_backend_list.front()->next);

                    }

                }
                _now_size =_now_size+value.size() - flag->second->data.size();
                flag->second->data = value;
                _backend_list.set_end(flag->second);
                return true;
            }
            else{
            return false;
        }
    }


// See MapBasedGlobalLockImpl.h
bool MapBasedGlobalLockImpl::PutIfAbsent(const std::string &key, const std::string &value) {
        std::lock_guard<std::mutex> lck(mut);
        size_t size_element = key.size() + value.size();
        if (size_element>_max_size){
            
            return false;
        }
        auto flag = _backend.find(key);
        if ((flag==_backend.end())&&(_now_size+size_element<=_max_size)){
            while(_now_size+value.size() - flag->second->data.size()>_max_size){
                std::string key_for_del = _backend_list.front()->key;
                _now_size-=(key_for_del.size()+_backend_list.front()->data.size());
                _backend.erase(key_for_del);
                _backend_list.remove(_backend_list.front());
            }
            _backend_list.push_back(value);
            _backend_list.end()->key = key;
            _backend[_backend_list.end()->key]=_backend_list.end();
            _now_size+=size_element;
            
            return true;

        }
        else{
            
            return false;
        }
    }

// See MapBasedGlobalLockImpl.h
bool MapBasedGlobalLockImpl::Set(const std::string &key, const std::string &value) {
        std::lock_guard<std::mutex> lck(mut);
        
        size_t size_element = key.size() + value.size();
        auto flag = _backend.find(key);
        if (size_element>_max_size) {
            
            return false;
        }

        if (flag!=_backend.end()){
            while(_now_size+value.size() - flag->second->data.size()>_max_size){
                if(flag->second!= _backend_list.front()){
                    std::string key_for_del = _backend_list.front()->key;
                    _now_size-=(key_for_del.size()+_backend_list.front()->data.size());
                    _backend.erase(key_for_del);
                    _backend_list.remove(_backend_list.front());
                }
                else{
                    std::string key_for_del = _backend_list.front()->next->key;
                    _now_size-=(key_for_del.size()+_backend_list.front()->next->data.size());
                    _backend.erase(key_for_del);
                    _backend_list.remove(_backend_list.front()->next);

                }

            }
            _now_size =_now_size+value.size() - flag->second->data.size();
            flag->second->data = value;
            _backend_list.set_end(flag->second);
            
            return true;
        }
        else{
            
            return false;
        }
    }

// See MapBasedGlobalLockImpl.h
bool MapBasedGlobalLockImpl::Delete(const std::string &key) {
        std::lock_guard<std::mutex> lck(mut);
        
        auto flag = _backend.find(key);
        if (flag!=_backend.end()){
            size_t size_element = flag->second->data.size()+flag->second->key.size();
            _now_size -=size_element;
            _backend.erase(key);
            _backend_list.remove(flag->second);
            
            return true;
        }
        else{
            
            return false;
        }
    }

// See MapBasedGlobalLockImpl.h
bool MapBasedGlobalLockImpl::Get(const std::string &key, std::string &value) const {
        std::lock_guard<std::mutex> lck(mut);
        auto flag = _backend.find(key);
        if (flag ==_backend.end()){
            
            return false;
        }
        else{

            value= flag->second->data;
            _backend_list.set_end(flag->second);
            
            return true;
        }
    }
} // namespace Backend
} // namespace Afina
