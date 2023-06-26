如果读者想看看第一段就行。本篇笔记未为读者考虑。  
了解当指令中outport为0时channel中的细节,主要是时序电路的状态和Channel中数据的输出 结论是outport控制当前tile中的channel的数据入列，和数据来源的tile中的channel的数据出列。当outport为0时，当前tile对应的channel不会接受数据，其输出为队列头的数据，但这个数据如果被下一个tile的某个channel接收，则本tile中当前channel中保持的数据会出列。  
指令首先作用于Tile中的Crossbar，看Crossbar中的代码，Crossbar输出给channel的send_data_en，会在指令的outport部分大于0，且对应的channel输入Crossbar的send_data_rdy为1的情况下置1，其余情况下置0。
先默认send_data_rdy总是为1，先看send_data_en的作用，输入channel后变为信号recv_en，recv_en信号对queues_enq_en起控制作用，**所以在outport部分为0时，channel中的数据队列是不能入列的，不会有新数据加入队列**。
**channel的输出，直接连接到了queues_deq_ret，即队列的输出，可以理解channel是始终保持输出，输出的是队列头的数据。**
此时我有个疑问，如果下个Tile要获取此Tile某个channel中的数据，队列中的数据是否会出列？
在channel中，queues_deq_en = send__rdy && queues__deq__rdy，看这两个信号的含义,
send_rdy来自输入tile的tile_send_data_rdy, tile_send_data_rdy来自周围tile的tile_recv_data_rdy,这个信号来自周围tile中的crossbar，在crossbar中这个信号是输出信号recv_data_rdy,当corssbar接收到的指令指定某个输入会被channel接收时，会将对应的recv_data_rdy设置成该channel的send_data_rdy,这个信号我们前面也提到过，并假设它总是1，现在需要直面这个问题了，这个send_data_rdy信号来外部输入,是由channel输出给crossbar,在channel中这个输出信号叫recv_rdy，这个信号来自channel中的队列enq_rdy，即数据可以入队，这个信号一般都是1，
接下来看queues_deq_rdy,这个信号来自本channel中的队列，一般在队列中有还有数据的时候都支持出列，所以可认为总为1。
**那么可以发现，只要下一个Tile中的某个channel要接受数据时，上一个Tile中channel中的数据就会出列。**