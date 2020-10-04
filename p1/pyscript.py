for t in [ 1, 2, 4, 6, 8 ]:
    print "NUMT = %d" % t
    for s in [ 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 3072, 4096 ]:
        print "NUMTRIALS = %d" % s
        cmd = "g++ -DNUMTRIALS=%d -DNUMT=%d proj1.cpp -o proj -lm -fopenmp" % ( s, t )
        os.system( cmd )
        cmd = "./proj"
        os.system( cmd )