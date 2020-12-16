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
      expect(hive.who).to eq '127.0.0.1:3001'
    end
    it 'proxies to its own redis' do
      expect(hive.proxy('info').grep(/role:/)).to eq ["role:master\r\n"]
    end
  end
  describe 'three nodes' do
    let(:workers) { 1 }

    it 'is has a leader' do
      expect(%w(127.0.0.1:3001 127.0.0.1:3002 127.0.0.1:3003)).to include hive.who
    end
    it 'proxies to redis master' do
      expect(hive.proxy('info').grep(/role:/)).to eq ["role:master\r\n"]
    end
  end
end
