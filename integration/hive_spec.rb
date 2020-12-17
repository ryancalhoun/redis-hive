require_relative 'hive'

describe 'redis-hive' do
  let(:hive) { Hive.new workers }
  before(:each) do
    hive.start!
  end
  after(:each) do
    hive.stop!
  end

  describe 'one node' do
    let(:workers) { 1 }

    it 'is its own leader' do
      expect(hive.leader).to eq '127.0.0.1:3001'
    end
    it 'proxies to its own redis' do
      expect(hive.redis_info['role']).to eq 'master'
    end
  end
  describe 'three nodes' do
    let(:workers) { 3 }

    it 'has three nodes' do
      expect(hive.who.values.size).to eq workers
    end
    it 'has a leader' do
      expect(%w(127.0.0.1:3001 127.0.0.1:3002 127.0.0.1:3003)).to include hive.leader
    end
    it 'all nodes proxy to redis master' do
      leader_redis_port = hive.leader.split(':').last.sub('3', '6')
      (1..workers).each do |i|
        info = hive.redis_info i
        expect(info['role']).to eq 'master'
        expect(info['tcp_port']).to eq leader_redis_port
      end
    end

    describe 'lose a worker' do
      describe 'node' do
      end
      describe 'redis' do
      end
    end
  end
end
