#pragma once

#include "CubicSDRDefs.h"
#include "ThreadQueue.h"
#include "IOThread.h"
#include <algorithm>

template<class InputDataType = ReferenceCounter, class OutputDataType = ReferenceCounter>
class VisualProcessor {
public:
	virtual ~VisualProcessor() {

	}
    
    bool isInputEmpty() {
        std::lock_guard < std::recursive_mutex > busy_lock(busy_update);

        return input->empty();
    }
    
    bool isOutputEmpty() {
        std::lock_guard < std::recursive_mutex > busy_lock(busy_update);

        for (outputs_i = outputs.begin(); outputs_i != outputs.end(); outputs_i++) {
            if ((*outputs_i)->full()) {
                return false;
            }
        }
        return true;
    }
    
    bool isAnyOutputEmpty() {
        std::lock_guard < std::recursive_mutex > busy_lock(busy_update);

        for (outputs_i = outputs.begin(); outputs_i != outputs.end(); outputs_i++) {
            if (!(*outputs_i)->full()) {
                return true;
            }
        }
        return false;
    }

    void setInput(ThreadQueue<InputDataType *> *vis_in) {
        std::lock_guard < std::recursive_mutex > busy_lock(busy_update);
        input = vis_in;
        
    }
    
    void attachOutput(ThreadQueue<OutputDataType *> *vis_out) {
        // attach an output queue
        std::lock_guard < std::recursive_mutex > busy_lock(busy_update);
        outputs.push_back(vis_out);
       
    }
    
    void removeOutput(ThreadQueue<OutputDataType *> *vis_out) {
        // remove an output queue
        std::lock_guard < std::recursive_mutex > busy_lock(busy_update);

        typename std::vector<ThreadQueue<OutputDataType *> *>::iterator i = std::find(outputs.begin(), outputs.end(), vis_out);
        if (i != outputs.end()) {
            outputs.erase(i);
        }
      
    }
    
    void run() {
        
        std::lock_guard < std::recursive_mutex > busy_lock(busy_update);

        if (input && !input->empty()) {
            process();
        }
       
    }
    
protected:
    virtual void process() {
        // process inputs to output
        // distribute(output);
    }

    void distribute(OutputDataType *output) {
        // distribute outputs
        std::lock_guard < std::recursive_mutex > busy_lock(busy_update);

        output->setRefCount(outputs.size());
        for (outputs_i = outputs.begin(); outputs_i != outputs.end(); outputs_i++) {
        	if ((*outputs_i)->full()) {
        		output->decRefCount();
        	} else {
        		(*outputs_i)->push(output);
        	}
        }
    }

    ThreadQueue<InputDataType *> *input;
    std::vector<ThreadQueue<OutputDataType *> *> outputs;
	typename std::vector<ThreadQueue<OutputDataType *> *>::iterator outputs_i;

    //protects input and outputs, must be recursive because ao reentrance
    std::recursive_mutex busy_update;
};


template<class OutputDataType = ReferenceCounter>
class VisualDataDistributor : public VisualProcessor<OutputDataType, OutputDataType> {
protected:
    void process() {
        while (!VisualProcessor<OutputDataType, OutputDataType>::input->empty()) {
            if (!VisualProcessor<OutputDataType, OutputDataType>::isAnyOutputEmpty()) {
                return;
            }
        	OutputDataType *inp;
        	VisualProcessor<OutputDataType, OutputDataType>::input->pop(inp);
            if (inp) {
            	VisualProcessor<OutputDataType, OutputDataType>::distribute(inp);
            }
        }
    }
};


template<class OutputDataType = ReferenceCounter>
class VisualDataReDistributor : public VisualProcessor<OutputDataType, OutputDataType> {
protected:
    void process() {
        while (!VisualProcessor<OutputDataType, OutputDataType>::input->empty()) {
            if (!VisualProcessor<OutputDataType, OutputDataType>::isAnyOutputEmpty()) {
                return;
            }
            OutputDataType *inp;
            VisualProcessor<OutputDataType, OutputDataType>::input->pop(inp);
            
            if (inp) {
                OutputDataType *outp = buffers.getBuffer();
                (*outp) = (*inp);
                inp->decRefCount();
                VisualProcessor<OutputDataType, OutputDataType>::distribute(outp);
            }
        }
    }
    ReBuffer<OutputDataType> buffers;
};
