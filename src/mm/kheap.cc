// /**
//  * @file kheap.cc
//  *
//  * @section LICENSE
//  *
//  * Copyright (C) 2013  Ryan Bunker
//  *
//  * This program is free software: you can redistribute it and/or modify
//  * it under the terms of the GNU General Public License as published by
//  * the Free Software Foundation, either version 3 of the License, or
//  * (at your option) any later version.
//  *
//  * This program is distributed in the hope that it will be useful,
//  * but WITHOUT ANY WARRANTY; without even the implied warranty of
//  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  * GNU General Public License for more details.
//  *
//  * You should have received a copy of the GNU General Public License
//  * along with this program.  If not, see [http://www.gnu.org/licenses/].
//  *
//  * @section DESCRIPTION
//  *
//  */

// #include "mm/kheap.h"

// #include <cstring>

// #include "mm/page_allocator.h"
// #include "mm/paging.h"
// #include "sys/addressing.h"
// #include "sys/kernel.h"

// namespace alloc {

// /**
//  * The amount of space to use to store the heap index.
//  */
// const uint32_t kHeapIndexSize = 0x4000;

// /**
//  * The minimum size of the heap.
//  */
// const size_t kHeapMinSize = 0x70000;

// KHeap::KHeap(void *start_address, void *end_address, void *max_address,
//              bool supervisor, bool readonly)
//     : start_address_(reinterpret_cast<uint32_t>(start_address)),
//       end_address_(reinterpret_cast<uint32_t>(end_address)),
//       max_address_(reinterpret_cast<uint32_t>(max_address)),
//       supervisor_(supervisor), readonly_(readonly),
//       index_(static_cast<KHeap::Header **>(start_address), kHeapIndexSize) {
//   // assert that the start and end address are page aligned
//   ASSERT(start_address_ % paging::kPageSize == 0);
//   ASSERT(end_address_ % paging::kPageSize == 0);

//   // Shift the start address forward to resemble where we can start putting data
//   start_address_ += sizeof(KHeap::Header) * kHeapIndexSize;

//   // Make sure the start address is page-aligned
//   if ((start_address_ & paging::kPageAlignMask) != 0) {
//     start_address_ &= paging::kPageAlignMask;
//     start_address_ += paging::kPageSize;
//   }

//   // We start off with one large hole in the index
//   KHeap::Header *hole = reinterpret_cast<KHeap::Header *>(start_address_);
//   hole->size = end_address_ - start_address_;
//   hole->magic = Magic::kValue;
//   hole->is_hole = true;
//   index_.Insert(hole);
// }

// void *KHeap::Allocate(size_t size, bool align) {
//   // make sure we take the size of header/footer into account
//   size_t new_size = size + sizeof(KHeap::Header) + sizeof(KHeap::Footer);
//   // find the smallest hole that will fit
//   int iterator = FindSmallestHole(new_size, align);

//   if (iterator == -1) {
//     // if we didn't find a suitable hole save some previous data
//     uint32_t old_length = end_address_ - start_address_;
//     uint32_t old_end_address = end_address_;

//     // we need to allocate some more space
//     Expand(old_length + new_size);
//     uint32_t new_length = end_address_ - start_address_;

//     // find the endmost header (not endmost in size, but in location)
//     // index and value of the endmost header found so far
//     int idx = -1;
//     uint32_t value = 0x0;
//     for (iterator = 0; iterator < index_.size(); ++iterator) {
//       uint32_t tmp = reinterpret_cast<uint32_t>(index_[iterator]);
//       if (tmp <= value)
//         continue;
//       value = tmp;
//       idx = iterator;
//     }

//     // if we didn't find ANY headers, we need to add one
//     if (idx == -1) {
//       KHeap::Header *header =
//           reinterpret_cast<KHeap::Header *>(old_end_address);
//       header->magic = Magic::kValue;
//       header->size = new_length - old_length;
//       header->is_hole = true;

//       KHeap::Footer *footer = reinterpret_cast<KHeap::Footer *>(
//           old_end_address + header->size - sizeof(KHeap::Footer));
//       footer->magic = Magic::kValue;
//       footer->header = header;
//       index_.Insert(header);
//     } else {
//       // the last header needs adjusting
//       KHeap::Header *header = index_[idx];
//       header->size += new_length - old_length;
//       // rewrite the footer
//       KHeap::Footer *footer = reinterpret_cast<KHeap::Footer *>(
//           reinterpret_cast<uint32_t>(header) + header->size -
//           sizeof(KHeap::Footer));
//       footer->header = header;
//       footer->magic = Magic::kValue;
//     }

//     // we now have enough space. Recurse, and call the function again
//     return Allocate(size, align);
//   } // if no hole found

//   KHeap::Header *orig_hole_header = index_[iterator];
//   uint32_t orig_hole_pos = reinterpret_cast<uint32_t>(orig_hole_header);
//   uint32_t orig_hole_size = orig_hole_header->size;
//   // here we work out if we should split the hole we found into two parts
//   // is the original hole size - requested hole size less than the overhead for
//   // adding a new hole?
//   size_t overhead = sizeof(KHeap::Header) + sizeof(KHeap::Footer);
//   if (orig_hole_size - new_size < overhead) {
//     // then just increase the requested size to the size of the hole we found
//     size += orig_hole_size - new_size;
//     new_size = orig_hole_size;
//   }

//   // if we need to page-align the data, do it now and make a new hole in front
//   // of our block.
//   if (align && (orig_hole_pos & paging::kPageAlignMask)) {
//     uint32_t new_location = orig_hole_pos + paging::kPageSize -
//                             (orig_hole_pos & 0xFFF) - sizeof(KHeap::Header);

//     KHeap::Header *hole_header =
//         reinterpret_cast<KHeap::Header *>(orig_hole_pos);
//     hole_header->size =
//         paging::kPageSize - (orig_hole_pos & 0xFFF) - sizeof(KHeap::Header);
//     hole_header->magic = Magic::kValue;
//     hole_header->is_hole = true;

//     KHeap::Footer *hole_footer =
//         reinterpret_cast<KHeap::Footer *>(new_location - sizeof(KHeap::Footer));
//     hole_footer->magic = Magic::kValue;
//     hole_footer->header = hole_header;
//     orig_hole_pos = new_location;
//     orig_hole_size = orig_hole_size - hole_header->size;
//   } else {
//     // else we don't need this hole any more, delete it from the index
//     index_.Remove(iterator);
//   }

//   // overwrite the original header...
//   KHeap::Header *block_header =
//       reinterpret_cast<KHeap::Header *>(orig_hole_pos);
//   block_header->magic = Magic::kValue;
//   block_header->is_hole = false;
//   block_header->size = new_size;

//   // ...and the footer
//   KHeap::Footer *block_footer = reinterpret_cast<KHeap::Footer *>(
//       orig_hole_pos + sizeof(KHeap::Header) + size);
//   block_footer->magic = Magic::kValue;
//   block_footer->header = block_header;

//   // we may need to write a new hole after the allocated block
//   // we do this only if the new hole would have positive size...
//   if (orig_hole_size - new_size > 0) {
//     KHeap::Header *hole_header = reinterpret_cast<KHeap::Header *>(
//         orig_hole_pos + sizeof(KHeap::Header) + size + sizeof(KHeap::Footer));
//     hole_header->magic = Magic::kValue;
//     hole_header->is_hole = true;
//     hole_header->size = orig_hole_size - new_size;

//     KHeap::Footer *hole_footer = reinterpret_cast<KHeap::Footer *>(
//         reinterpret_cast<uint32_t>(hole_header) + orig_hole_size - new_size -
//         sizeof(KHeap::Footer));
//     if (reinterpret_cast<uint32_t>(hole_footer) < end_address_) {
//       hole_footer->magic = Magic::kValue;
//       hole_footer->header = hole_header;
//     }

//     // put the new hole in the index
//     index_.Insert(hole_header);
//   }

//   // ...and we're done!
//   return reinterpret_cast<void *>(reinterpret_cast<uint32_t>(block_header) +
//                                   sizeof(KHeap::Header));
// }

// void KHeap::Free(void *p) {
//   // exit gracefully for null pointers
//   if (p == nullptr)
//     return;

//   // get the header and footer associated with this pointer
//   KHeap::Header *header = reinterpret_cast<KHeap::Header *>(
//       reinterpret_cast<uint32_t>(p) - sizeof(KHeap::Header));
//   KHeap::Footer *footer =
//       reinterpret_cast<KHeap::Footer *>(reinterpret_cast<uint32_t>(header) +
//                                         header->size - sizeof(KHeap::Footer));

//   // sanity checks
//   ASSERT(header->magic == Magic::kValue);
//   ASSERT(footer->magic == Magic::kValue);

//   // make us a hole
//   header->is_hole = true;

//   // do we want to add this header into the 'free holes' index?
//   bool do_add = true;

//   // == unify left ==
//   // if the thing immediately to the left of us is a footer...
//   KHeap::Footer *test_footer = reinterpret_cast<KHeap::Footer *>(
//       reinterpret_cast<uint32_t>(header) - sizeof(KHeap::Footer));
//   if (test_footer->magic == Magic::kValue && test_footer->header->is_hole) {
//     // cache our current size
//     uint32_t cache_size = header->size;
//     // rewrite our header with the new one
//     header = test_footer->header;
//     // rewrite our footer to point to the new header
//     footer->header = header;
//     // change the size
//     header->size += cache_size;
//     // since this header is already in the index, we don't want to add it again
//     do_add = false;
//   }

//   // == unify right ==
//   // if the thing immediately to the right of us is a header...
//   KHeap::Header *test_header = reinterpret_cast<KHeap::Header *>(
//       reinterpret_cast<uint32_t>(footer) + sizeof(KHeap::Footer));
//   if (test_header->magic == Magic::kValue && test_header->is_hole) {
//     // increase our size
//     header->size += test_header->size;
//     // rewrite it's footer to point to our header
//     test_footer = reinterpret_cast<KHeap::Footer *>(
//         reinterpret_cast<uint32_t>(test_header) + test_header->size -
//         sizeof(KHeap::Footer));
//     footer = test_footer;
//     // find and remove this header from the index
//     int iterator = 0;
//     while ((iterator < index_.size()) && (index_[iterator] != test_header))
//       iterator++;

//     // make sure we actually found the item
//     ASSERT(iterator < index_.size());
//     // remove it
//     index_.Remove(iterator);
//   }

//   // if the footer location is the end address, we can contract
//   if (reinterpret_cast<uint32_t>(footer) + sizeof(KHeap::Footer) ==
//       end_address_) {
//     uint32_t old_length = end_address_ - start_address_;
//     uint32_t new_length =
//         Contract(reinterpret_cast<uint32_t>(header) - start_address_);
//     // check how big we will be after resizing
//     if (header->size - (old_length - new_length) > 0) {
//       // we will still exist, so resize us
//       header->size -= old_length - new_length;
//       footer = reinterpret_cast<KHeap::Footer *>(
//           reinterpret_cast<uint32_t>(header) + header->size -
//           sizeof(KHeap::Footer));
//       footer->magic = Magic::kValue;
//       footer->header = header;
//     } else {
//       // we will no longer exist :( so remove us from the index
//       int iterator = 0;
//       while ((iterator < index_.size()) && (index_[iterator] != test_header))
//         iterator++;
//       // if we didn't find ourselves, we have nothing to remove
//       if (iterator < index_.size())
//         index_.Remove(iterator);
//     }
//   }

//   if (do_add)
//     index_.Insert(header);
// }

// int32_t KHeap::FindSmallestHole(size_t size, bool align) {
//   // find the smallest hole that will fit
//   int iterator = 0;
//   while (iterator < index_.size()) {
//     KHeap::Header *header = index_[iterator];
//     // if the user has requested the memory be page-aligned
//     if (align) {
//       // page-align the starting point of this header
//       uint32_t location = reinterpret_cast<uint32_t>(&header);
//       int32_t offset = 0;
//       if (((location + sizeof(KHeap::Header)) & paging::kPageAlignMask) != 0)
//         offset = paging::kPageSize -
//                  (location + sizeof(KHeap::Header)) % paging::kPageSize;
//       int32_t hole_size = static_cast<int32_t>(header->size) - offset;
//       // can we fit now?
//       if (hole_size >= static_cast<int32_t>(size))
//         break;
//     } else if (header->size >= size) {
//       break;
//     }
//     ++iterator;
//   }

//   // why did the loop exit?
//   if (iterator == index_.size())
//     return -1; // we got to the end and didn't find anything
//   else
//     return iterator;
// }

// void KHeap::Expand(size_t new_size) {
//   ASSERT(new_size > end_address_ - start_address_);

//   // get the nearest following page boundary
//   if ((new_size & paging::kPageAlignMask) != 0) {
//     new_size &= paging::kPageAlignMask;
//     new_size += paging::kPageSize;
//   }
//   // make sure we are not over-reaching ourselves
//   ASSERT(start_address_ + new_size <= max_address_);

//   // this should always be on a page boundary
//   size_t old_size = end_address_ - start_address_;
//   uint32_t frames_needed = (new_size - old_size) / paging::kPageSize;
//   paging::Page *first_frame =
//       paging::PageAllocator::instance().AllocatePages(frames_needed);
//   for (uint32_t i = 0; i < frames_needed; ++i, ++first_frame)
//     paging::PageDirectory::current_directory()->MapPage(
//         first_frame,
//         reinterpret_cast<void *>(end_address_ + i * paging::kPageSize));

//   end_address_ = start_address_ + new_size;
// }

// size_t KHeap::Contract(size_t new_size) {
//   ASSERT(new_size < end_address_ - start_address_);

//   // get the nearest following page boundary
//   if (new_size & paging::kPageSize) {
//     new_size &= paging::kPageSize;
//     new_size += paging::kPageSize;
//   }

//   // don't contract too far!
//   if (new_size < kHeapMinSize)
//     new_size = kHeapMinSize;

//   size_t old_size = end_address_ - start_address_;

//   for (uint32_t i = old_size - paging::kPageSize; new_size < i;
//        i -= paging::kPageSize) {
//     paging::Page *frame = paging::PageDirectory::current_directory()->UnmapPage(
//         reinterpret_cast<void *>(i));
//     paging::PageAllocator::instance().FreePage(*frame);
//   }

//   end_address_ = start_address_ + new_size;
//   return new_size;
// }

// KHeap::OrderedHeaderArray::OrderedHeaderArray(uint32_t max_size)
//     : array_(new KHeap::Header *[max_size]), size_(0), max_size_(max_size) {
//   memset(array_, 0, max_size * sizeof(KHeap::Header *));
// }

// KHeap::OrderedHeaderArray::OrderedHeaderArray(KHeap::Header **addr,
//                                               uint32_t max_size)
//     : array_(addr), size_(0), max_size_(max_size) {
//   memset(array_, 0, max_size * sizeof(KHeap::Header *));
// }

// KHeap::OrderedHeaderArray::~OrderedHeaderArray() { delete[] array_; }

// bool KHeap::OrderedHeaderArray::Insert(const KHeap::Header *item) {
//   if (size_ == max_size_)
//     return false;

//   int iterator = 0;
//   while (iterator < size_ && array_[iterator]->size < item->size)
//     iterator++;
//   if (iterator == size_) { // just add at the end of the array.
//     array_[size_++] = const_cast<KHeap::Header *>(item);
//   } else {
//     KHeap::Header *tmp = array_[iterator];
//     array_[iterator] = const_cast<KHeap::Header *>(item);
//     while (iterator < size_) {
//       ++iterator;
//       KHeap::Header *tmp2 = array_[iterator];
//       array_[iterator] = tmp;
//       tmp = tmp2;
//     }
//     ++size_;
//   }
//   return true;
// }

// KHeap::Header *KHeap::OrderedHeaderArray::operator[](const int index) {
//   return array_[index];
// }

// void KHeap::OrderedHeaderArray::Remove(int i) {
//   for (; i < size_; ++i)
//     array_[i] = array_[i + 1];
//   --size_;
// }

// } // namespace alloc
