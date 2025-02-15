#ifndef LIVE_DATA_RATE_MANAGER_H
#define LIVE_DATA_RATE_MANAGER_H

namespace p2sp
{
    class LiveDataRateManager
    {
    public:
        void Start(const vector<RID>& rids, const vector<boost::uint32_t>& data_rate_s);
        RID GetCurrentRID();
        
        bool SwitchToHigherDataRateIfNeeded(boost::uint32_t rest_time_in_seconds);
        bool SwitchToLowerDataRateIfNeeded(boost::uint32_t rest_time_in_seconds);

        boost::uint32_t GetLastDataRatePos() const;
        boost::uint32_t GetCurrentDataRatePos() const;

        const vector<boost::uint32_t>& GetDataRates() const;

        const vector<RID>& GetRids() const;

    private:
        vector<RID> rid_s_;
        framework::timer::TickCounter timer_;
        boost::uint32_t current_data_rate_pos_;
        boost::uint32_t last_data_rate_pos_;
        vector<boost::uint32_t> data_rate_s_;
    };
}

#endif