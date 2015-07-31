#!/usr/bin/env python
# -*- coding: utf-8 -*-

from __future__ import print_function

import sys
from colorama import init, Fore, Back, Style
from loganalyze import *
import subprocess
import time
import glob
import os
import time
import shutil
from collections import Counter
import platform

if __name__ == '__main__':
	init() #colorama
	print(Fore.CYAN + "--- Start script ---" + Fore.RESET)
	
	logdir = 'output/'
	
	if platform.system() == 'Windows':
		process_path = '../x64/Release/cache-attack'
	else:
		process_path = '../out/bin/cache-attack'
	
	if 'r' in map(str.lower, sys.argv):
		if not os.path.exists(logdir):
			os.makedirs(logdir)
		else:
			shutil.rmtree(logdir)
			os.makedirs(logdir)
		
		def log_path():
			return logdir + 'log-' + str(int(time.time() * 1000)) + '.txt'
		
		list = [x * 1000 for x in range(1, 100)] * 100
		# list = [50000] * 100
		# list_max = len(list)
		# list_count = Counter(list)
		# sums = {x:0.0 for x in list}
		
		print(Fore.CYAN + "--- Running ---" + Fore.RESET)
		
		for cnt in list:
			path = log_path()
			while os.path.exists(path):
				time.sleep(0)
				path = log_path()
			subprocess.check_call([process_path, 'o' + path, 'c' + str(cnt)])
			
			# attack_log = AttackLog(path)
			# attack_log.load()
			# sums[attack_log.text_max] += math.log(attack_log.last_rk_candidate_mult(), 2)
			
			# os.remove(path)

	if 'a' in map(str.lower, sys.argv):
		import numpy as np
		from scipy.stats import sem
		
		print(Fore.CYAN + "--- Analyzing ---" + Fore.RESET)
		
		result = {}
		attack_log = AttackLog()
		
		for path in glob.glob(logdir + 'log-*.txt'):
			attack_log.load(path)
			#attack_log.dump()
			
			if not attack_log.text_max in result:
				result[attack_log.text_max] = []
			cand = math.log(attack_log.last_rk_candidate_mult(), 2)
			result[attack_log.text_max].append(cand)
			# sums[attack_log.text_max] += math.log(attack_log.last_rk_candidate_mult(), 2)
			
		print(Fore.CYAN + "--- Result ---" + Fore.RESET)
		
		time_str = str(int(time.time() * 1000))
		result_file1 = open('result1-'  + time_str + '.txt', 'w')
		# result_file2 = open('result2-'  + time_str + '.txt', 'w')
		# for k, v in sorted(sums.items()):
			# #print("Text(%4d):2^%f" % (k, v / list_count[k]))
			# result_file1.write(str(k) + " " + str(v / list_count[k]) + "\n")
		for k, v in sorted(result.items()):
			data = np.array(v)
			result_file1.write(str(k))
			result_file1.write(" " + str(np.mean(data)))
			result_file1.write(" " + str(np.var(data)))
			result_file1.write(" " + str(np.std(data)))
			result_file1.write(" " + str(sem(data)))
			result_file1.write(" " + str(np.amax(data)))
			result_file1.write(" " + str(np.amin(data)))
			result_file1.write(" " + str(np.median(data)))
			result_file1.write("\n")
			# for value in v:
				# result_file2.write(str(k) + " " + str(value) + "\n")
		result_file1.close()
		# result_file2.close()
