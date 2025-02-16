require_relative 'hive'
require 'timeout'

describe 'redis-hive' do
  let(:hive) { Hive.new workers, auth: auth }
  before(:each) do
    hive.start!
  end
  after(:each) do
    hive.stop!
  end

  describe 'one node' do
    let(:workers) { 1 }

    describe 'no auth' do
      let(:auth) {}
      it 'is its own leader' do
        expect(hive.leader).to eq '127.0.0.1:3001'
      end
      it 'proxies to its own redis' do
        expect(hive.redis_info['role']).to eq 'master'
      end
    end
    describe 'with auth' do
      let(:auth) { 'foo' }
      it 'uses a password' do
        expect(hive.direct_redis_command('config', 'get', 'requirepass')).to eq ['requirepass', auth]
        expect(hive.proxy_redis_command('config', 'get', 'requirepass')).to eq ['requirepass', auth]
      end
    end
  end
  describe 'three nodes' do
    let(:workers) { 3 }

    describe 'no auth' do
      let(:auth) {}
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
        let(:lost) { ([1, 2, 3] - [hive.leader[-1].to_i]).first }
        before(:each) do
          hive.proxy_redis_command("set", "foo", 42)
        end
        describe 'node' do
          it 'all redis replicas are working' do
            hive.stop(node: [lost])

            (1..workers).each do |i|
              expect(hive.direct_redis_command("get", "foo", i: i)).to eq "42"
            end
          end
        end
        describe 'redis' do
          it 'the replica is gone' do
            hive.stop(redis: [lost])

            expect { hive.direct_redis_command("get", "foo", i: lost) }.to raise_error Redis::CannotConnectError

            hive.start!

            Timeout::timeout(5) do
              until hive.direct_redis_command("get", "foo", i: lost) == "42"
                sleep 0.5
              end
            end
          end
          it 'all remaining replicas are working' do
            hive.stop(redis: [lost])

            ((1..workers).to_a - [lost]).each do |i|
              expect(hive.direct_redis_command("get", "foo", i: i)).to eq "42"
            end
          end
        end
      end
      describe 'lose a leader' do
        let(:leader) { hive.leader[-1].to_i }
        before(:each) do
          hive.proxy_redis_command("set", "foo", 42)
        end
        describe 'node' do
          it 'elects a new leader' do
            expect(hive.direct_who(leader)['s']).to eq 'L'

            hive.stop(node: [leader])

            Timeout::timeout(15) do
              until ((1..workers).to_a - [leader]).map {|i| hive.direct_who(i) }.select {|p| p['s'] == 'L'}.size == 1
                sleep 0.5
              end
            end

            hive.start!
            expect(hive.direct_who(leader)['s']).to eq 'F'

          end
          it 'all redis replicas are working' do
            hive.stop(node: [leader])

            (1..workers).each do |i|
              expect(hive.direct_redis_command("get", "foo", i: i)).to eq "42"
            end
          end
        end
        describe 'redis' do
          it 'elects a new leader' do
            expect(hive.direct_redis_command("info", i: leader)['role']).to eq "master"
            hive.stop(redis: [leader])

            expect { hive.direct_redis_command("get", "foo", i: leader) }.to raise_error Redis::CannotConnectError

            hive.start!

            Timeout::timeout(5) do
              until hive.direct_redis_command("info", i: leader)['role'] == "slave"
                sleep 0.5
              end
            end

            expect(hive.direct_redis_command("get", "foo", i: leader)).to eq "42"
          end
          it 'all remaining replicas are working' do
            hive.stop(redis: [leader])

            ((1..workers).to_a - [leader]).each do |i|
              expect(hive.direct_redis_command("get", "foo", i: i)).to eq "42"
            end
          end
        end
      end
    end
    describe 'with auth' do
      let(:auth) { 'foo' }
      it 'has three nodes' do
        expect(hive.who.values.size).to eq workers
      end
      it 'uses a password' do
        expect(hive.direct_redis_command('config', 'get', 'requirepass')).to eq ['requirepass', auth]
        expect(hive.proxy_redis_command('config', 'get', 'requirepass')).to eq ['requirepass', auth]
      end
      it 'all nodes use redis master auth' do
        (1..workers).each do |i|
          expect(hive.direct_redis_command('config', 'get', 'masterauth')).to eq ['masterauth', auth]
        end
      end
    end
  end
end
