#pragma once
#include "../../Base/BaseGUIWidget.h"
#include "../../ThirdParty/spdlog/sinks/base_sink.h"

#include "../../ThirdParty/spdlog/details/null_mutex.h"
#include <mutex>

template<typename Mutex>
class CoreImGuiLoggerSink : public spdlog::sinks::base_sink <Mutex>
{
public:
    CoreImGuiLoggerSink() = default;

protected:
    void sink_it_(const spdlog::details::log_msg& msg) override
    {
        spdlog::memory_buf_t formatted;
        spdlog::sinks::base_sink<Mutex>::formatter_->format(msg, formatted);
        // TBD
        fmt::to_string(formatted);
    }

    void flush_() override
    {
        // TBD
    }
};

using CoreImGuiLoggerSinkMt = CoreImGuiLoggerSink<std::mutex>;
using CoreImGuiLoggerSinkSt = CoreImGuiLoggerSink<spdlog::details::null_mutex>;

class CoreLoggerWidget : public BaseGUIWidget
{
public:
	CoreLoggerWidget();
	void Update();
	void Start(ImGuiID& dockspace_id);
};

