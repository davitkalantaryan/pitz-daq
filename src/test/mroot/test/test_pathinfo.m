function test = test_reformat

	% create testcase object
	test = munit_testcase;
	
	% create structure of constraint objects for assertions
	c = munit_constraint;
	
	% fixture
	tdir = [fileparts(which('test_pathinfo')) filesep];
	
	% test functions
	
	% ip  = input
	% r_  = reference value
	% a_  = actual value
	
	% not a ROOT file
	function test_00
		ip = [tdir 'matlab.mat'];
		
		 r_ofile = struct('type', -1, 'path', [tdir 'matlab.mat']);
		 r_ifile = struct('path', {});
		[a_ofile, a_ifile] = pathinfo(ip);
		
		test.assert(c.same(r_ofile, a_ofile));
		test.assert(isempty(a_ifile));
	end
	
	% TFile
	function test_01
		ip = [tdir 'matlab.root'];
		
		 r_ofile = {0 [tdir 'matlab.root']};
		[a_ofile, a_ifile] = pathinfo(ip);
		
		test.assert(c.same(r_ofile, a_ofile));
		test.assert(isempty(a_ifile));
	end
	
	% TFile without extension; exists
	function test_02
		ip = [tdir 'matlab'];
		
		 r_ofile = {0 [tdir 'matlab']};
		[a_ofile, a_ifile] = pathinfo(ip);
		
		test.assert(c.same(r_ofile, a_ofile));
		test.assert(isempty(a_ifile));
	end
	
	% TFile without extension; not exists
	function test_03
		ip = [tdir 'ratlab'];
		
		 r_ofile = {0 [tdir 'ratlab.root']};
		[a_ofile, a_ifile] = pathinfo(ip);
		
		test.assert(c.same(r_ofile, a_ofile));
		test.assert(isempty(a_ifile));
	end
	
	% TFile with inner path
	function test_04
		ip = [tdir 'matlab.root'];
		
		if isunix()
			ip = [ip '#foo/bar'];
		else
			ip = [ip '#foo\bar'];
		end
		
		 r_ofile = {0 [tdir 'matlab.root']};
		 r_ifile = {'foo' 'bar'};
		[a_ofile, a_ifile] = pathinfo(ip);
		
		test.assert(c.same(r_ofile, a_ofile));
		test.assert(c.same(r_ifile, a_ifile));
	end
	
	% TXMLFile
	function test_05
		ip = [tdir 'matlab.xml'];
		
		 r_ofile = {1 [tdir 'matlab.xml']};
		[a_ofile, a_ifile] = pathinfo(ip);
		
		test.assert(c.same(r_ofile, a_ofile));
		test.assert(isempty(a_ifile));
	end
	
	% TNetFile
	function test_06
		ip = 'root://usr:pwd@localhost/~usr/matlab.root';
		
		 r_ofile = {2 'root://localhost/~usr/matlab.root' 'usr' 'pwd'};
		[a_ofile, a_ifile] = pathinfo(ip);
		
		test.assert(c.same(r_ofile, a_ofile));
		test.assert(isempty(a_ifile));
	end

	% TNetFile without pwd
	function test_07
		ip = 'root://usr@localhost/~usr/matlab.root';
		
		 r_ofile = {2 'root://localhost/~usr/matlab.root' 'usr'};
		[a_ofile, a_ifile] = pathinfo(ip);
		
		test.assert(c.same(r_ofile, a_ofile));
		test.assert(isempty(a_ifile));
	end
	
	% TSQLFile
	function test_08
		ip = 'mysql://usr:pwd@localhost/matlab';
		
		 r_ofile = {3 'mysql://localhost/matlab' 'usr' 'pwd'};
		[a_ofile, a_ifile] = pathinfo(ip);
		
		test.assert(c.same(r_ofile, a_ofile));
		test.assert(isempty(a_ifile));
	end
	
	% I/O-Error while checking for TFile
	function test_09
		ip = [tdir 'ratlab.root'];
		
		% does not work on windows
		if isunix()
			% workaround: you can't check a locked file into SVN
			system(['chmod 000 ' ip]);
			test.assert_mlerror(@() pathinfo(ip), 'path:io');
			system(['chmod 644 ' ip]);
		end
	end
	
	% Directory not found
	function test_10
		if isunix()
			ip = [tdir 'foo/matlab.root'];
		else
			ip = [tdir 'foo\matlab.root'];
		end
		
		test.assert_mlerror(@() pathinfo(ip), 'path:not_found');
	end
	
	% Unsupported protocol
	function test_11
		ip = 'fish://usr:pwd@localhost/matlab';
		
		test.assert_mlerror(@() pathinfo(ip), 'path:proto');
	end

	% No login information for TSQLFile
	function test_12
		ip = 'mysql://localhost/matlab';
		
		test.assert_mlerror(@() pathinfo(ip), 'path:login');
	end

end
