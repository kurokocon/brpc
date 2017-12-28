// Copyright (c) 2014 Baidu, Inc.
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// 
//     http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// Authors: Zhao,Chong (zhaochong@bigo.sg)

#ifndef BRPC_POLICY_PRIMARY_LOAD_BALANCER_H
#define BRPC_POLICY_PRIMARY_LOAD_BALANCER_H

#include <vector>                                      // std::vector
#include <map>                                         // std::map
#include "butil/containers/doubly_buffered_data.h"
#include "brpc/load_balancer.h"

namespace brpc {
namespace policy {

// This LoadBalancer selects servers one by one, if the first server is available,
// selects the first server, else considers the second ...
class PrimaryLoadBalancer : public LoadBalancer {
public:
    bool AddServer(const ServerId& id);
    bool RemoveServer(const ServerId& id);
    size_t AddServersInBatch(const std::vector<ServerId>& servers);
    size_t RemoveServersInBatch(const std::vector<ServerId>& servers);
    int SelectServer(const SelectIn& in, SelectOut* out);
    void Feedback(const CallInfo& info);
    PrimaryLoadBalancer* New() const;
    void Destroy();
    void Describe(std::ostream& os, const DescribeOptions&);
    
private:
    // SocketStatus is used by double data buffer, data buffer need copy constructor,
    // But bvar types disallow copy and assign constructors, so use pointer here.
    struct SocketStatus {
        uint64_t last_remove_time;
        bvar::Adder<int> *count;
        bvar::Window<bvar::Adder<int>> *window_count;
        
        SocketStatus();

        // In the load balancer, Destroy is protected by double buffer mutex.
        // Destroy should be called before destruct method.
        void Destroy();
    };
    struct Servers {
        std::vector<ServerId> server_list;
        std::map<ServerId, size_t> server_map;
        std::map<SocketId, SocketStatus> status_map;
    };
    static bool Add(Servers& bg, const ServerId& id);
    static bool Change(Servers& bg, const SocketId& id, uint64_t time_us);
    static bool Remove(Servers& bg, const ServerId& id);
    static bool Destruct(Servers& bg);
    static size_t BatchAdd(Servers& bg, const std::vector<ServerId>& servers);
    static size_t BatchRemove(Servers& bg, const std::vector<ServerId>& servers);

    butil::DoublyBufferedData<Servers> _db_servers;
};

}  // namespace policy
} // namespace brpc


#endif  // BRPC_POLICY_PRIMARY_LOAD_BALANCER_H
