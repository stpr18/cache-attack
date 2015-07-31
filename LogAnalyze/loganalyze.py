#!/usr/bin/env python
# -*- coding: utf-8 -*-

from __future__ import print_function

import sys
import struct
import math
from colorama import init, Fore, Back, Style
from aes import Aes
from mean import OnlineMean
from util import Util

class LogReader:
	struct_format = 'I'
	struct_size = struct.calcsize(struct_format)

	def __init__(self, file_name):
		self.file = open(file_name, 'rb')
	
	def __del__(self):
		self.close()
	
	def close(self):
		if not self.file.closed:
			self.file.close()
	
	def read(self):
		data = self.file.read(LogReader.struct_size)
		return struct.unpack(LogReader.struct_format, data)[0]
	
	def read_list(self, size):
		return [self.read() for i in range(size)]
		
	def read_list2(self, size1, size2):
		return [self.read_list(size2) for i in range(size1)]
	
	def read_list3(self, size1, size2, size3):
		return [self.read_list2(size2, size3) for i in range(size1)]
	
	def read_tuple(self):
		return self.read(), self.read(), self.read(), self.read()
	
	def read_tuple_list(self, size):
		return [self.read_tuple() for i in range(size)]
	
class AttackLog:
	text_loop_max = 1
	int2q_func = [Util.int2q0, Util.int2q1, Util.int2q2, Util.int2q3]

	def __init__(self, file_name = None):
		self.file_name = file_name
	
	def load(self, file_name = None):
		if file_name is None:
			file_name = self.file_name
		#open
		reader = LogReader(file_name)
		#ReadCryptoHeader
		self.crypto_log_mode, self.round_key_size, self.cache_line_size, _ = reader.read_tuple()
		#ReadRoundKey
		self.rk = reader.read_list(self.round_key_size)
		#ReadAttackHeader
		self.attack_log_mode, self.text_max, self.counter_threshold, _ = reader.read_tuple()
		#ReadCryptoData
		if self.crypto_log_mode >= 2:
			self.table_input = reader.read_tuple_list(self.text_max)
			self.cipher = reader.read_tuple_list(self.text_max)
		#ReadCounter
		self.counter = reader.read_list3(16, 256, 4)
		#ReadLastRoundKeyCandidate
		if self.attack_log_mode >= 2:
			self.last_rk_candidate = reader.read_list2(16, 256)
		#ReadRecoveredLastRoundKey
		self.recovered_last_rk = reader.read_list(4)
		#ReadRecoveredSecretKey
		self.recovered_secret_key = reader.read_list(4)
		#close
		reader.close()
	
	def dump(self):
		#----------------------------------
		print(Fore.CYAN + "--- Round key ---" + Fore.RESET)
		for i, rk in enumerate(self.rk):
			print("%08X " % rk, end = "" if i % 4 != 3 else None)
		#----------------------------------
		print(Fore.CYAN + "--- Text max count ---" + Fore.RESET)
		print(self.text_max)
		#----------------------------------
		if self.crypto_log_mode >= 2:
			text_loop = self.text_max
			if text_loop > AttackLog.text_loop_max:
				text_loop = AttackLog.text_loop_max
			for text_cnt in range(text_loop):
				print(Fore.CYAN + "--- Text " + str(text_cnt) + " ---" + Fore.RESET)
				print("Cipher:%08X %08X %08X %08X" % (self.cipher[text_cnt][0], self.cipher[text_cnt][1], self.cipher[text_cnt][2], self.cipher[text_cnt][3]))
				for i in range(16):
					print("c%02d=%02X:" % (i, AttackLog.get_array_byte(self.cipher, text_cnt, i)), end = "")
					for j in range(4):
						print("T%d[%02X] " % (j, AttackLog.get_array_byte(self.table_input, text_cnt, i)), end = "")
					print()
		#----------------------------------
		if self.attack_log_mode >= 2:
			print(Fore.CYAN + "--- Key candidate ---" + Fore.RESET)
			last_rk_candidate = [self.get_last_rk_candidate(x) for x in range(16)]
			for i, candidate in enumerate(last_rk_candidate):
				print("K[%02d]:%-6d" % (i, candidate), end = "" if i % 4 != 3 else None)
			#print("Average:" + str(self.last_rk_candidate_average()))
			print("Average:2^" + str(math.log(self.last_rk_candidate_mult(last_rk_candidate), 2)))
		#----------------------------------
		print(Fore.CYAN + "--- Last round key recovery ---" + Fore.RESET)
		for i in range(16):
			part_key = AttackLog.int2q_func[i % 4](self.recovered_last_rk[i / 4])
			is_correct_key = part_key != self.correct_last_rk(i)
			print(Fore.RED + "%02X" % part_key + Fore.RESET if is_correct_key else "%02X" % part_key, end = "" if i % 4 != 3 else " ")
		print()
		#----------------------------------
		print(Fore.CYAN + "--- Secret key recovery ---" + Fore.RESET)
		for i in range(16):
			part_key = AttackLog.int2q_func[i % 4](self.recovered_secret_key[i / 4])
			is_correct_key = part_key != self.correct_secret_key(i)
			print(Fore.RED + "%02X" % part_key + Fore.RESET if is_correct_key else "%02X" % part_key, end = "" if i % 4 != 3 else " ")
		print()
		#----------------------------------
		# if self.crypto_log_mode >= 2:
			# print(Fore.CYAN + "--- Calc reload vector threshold ---" + Fore.RESET)
			# threshold = self.calc_threshold()
			# print("Max:" + str(threshold[0]))
			# print("Min:" + str(threshold[1]))
			# print("Ave:" + str(threshold[2]))
	
	@staticmethod
	def get_array_byte(cipher, text_cnt, i):
		return AttackLog.int2q_func[i % 4](cipher[text_cnt][i / 4])
	
	def get_last_rk_candidate(self, i):
		return Util.max_count(self.last_rk_candidate[i])
	
	def last_rk_candidate_average(self):
		return reduce(lambda x, y: x + Util.max_count(y), self.last_rk_candidate, 0) / 16.0
	
	def last_rk_candidate_mult(self, last_rk_candidate = None):
		if last_rk_candidate is None:
			last_rk_candidate = [self.get_last_rk_candidate(x) for x in range(16)]
		return reduce(lambda x, y: x * y, last_rk_candidate)
	
	def correct_last_rk(self, i):
		return AttackLog.int2q_func[i % 4](self.rk[40 + i / 4])
	
	def correct_secret_key(self, i):
		return AttackLog.int2q_func[i % 4](self.rk[i / 4])
	
	# def calc_threshold(self):
		# mean = OnlineMean()
		# max_val = 0
		# min_val = 256
		# for text_cnt in range(self.text_max):
			# for i in range(16):
				# for j in range(self.cache_line_size):
					# c = AttackLog.get_array_byte(self.cipher, text_cnt, i)
					# if self.correct_last_rk(i) == (Aes.sbox[j] ^ c):
						# v = self.counter[i][c][(i + 2) % 4]
						# mean.add_value(v)
						# max_val = max(max_val, v)
						# min_val = min(min_val, v)
		# return max_val, min_val, mean.value

if __name__ == '__main__':
	init() #colorama

	arg_len = len(sys.argv)
	if arg_len > 2:
		raise Exception("unexpected arguments")

	log_path = sys.argv[1] if arg_len == 2 else "../cache-attack/log.txt"

	attack_log = AttackLog(log_path)
	attack_log.load()
	attack_log.dump()
