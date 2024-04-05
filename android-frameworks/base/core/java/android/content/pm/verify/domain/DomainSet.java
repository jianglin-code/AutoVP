/*
 * Copyright (C) 2021 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package android.content.pm.verify.domain;

import android.annotation.NonNull;
import android.os.Parcel;
import android.os.Parcelable;

import com.android.internal.util.DataClass;

import java.util.Set;

/**
 * Wraps an input set of domains from the client process, to be sent to the server. Handles cases
 * where the data size is too large by writing data using {@link Parcel#writeBlob(byte[])}.
 *
 * @hide
 */
@DataClass(genParcelable = true, genAidl = true, genEqualsHashCode = true)
public class DomainSet implements Parcelable {

    @NonNull
    private final Set<String> mDomains;

    private void parcelDomains(@NonNull Parcel dest, @SuppressWarnings("unused") int flags) {
        DomainVerificationUtils.writeHostSet(dest, mDomains);
    }

    private Set<String> unparcelDomains(@NonNull Parcel in) {
        return DomainVerificationUtils.readHostSet(in);
    }



    // Code below generated by codegen v1.0.22.
    //
    // DO NOT MODIFY!
    // CHECKSTYLE:OFF Generated code
    //
    // To regenerate run:
    // $ codegen $ANDROID_BUILD_TOP/frameworks/base/core/java/android/content/pm/verify/domain
    // /DomainSet.java
    //
    // To exclude the generated code from IntelliJ auto-formatting enable (one-time):
    //   Settings > Editor > Code Style > Formatter Control
    //@formatter:off


    @DataClass.Generated.Member
    public DomainSet(
            @NonNull Set<String> domains) {
        this.mDomains = domains;
        com.android.internal.util.AnnotationValidations.validate(
                NonNull.class, null, mDomains);

        // onConstructed(); // You can define this method to get a callback
    }

    @DataClass.Generated.Member
    public @NonNull Set<String> getDomains() {
        return mDomains;
    }

    @Override
    @DataClass.Generated.Member
    public boolean equals(@android.annotation.Nullable Object o) {
        // You can override field equality logic by defining either of the methods like:
        // boolean fieldNameEquals(DomainSet other) { ... }
        // boolean fieldNameEquals(FieldType otherValue) { ... }

        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;
        @SuppressWarnings("unchecked")
        DomainSet that = (DomainSet) o;
        //noinspection PointlessBooleanExpression
        return true
                && java.util.Objects.equals(mDomains, that.mDomains);
    }

    @Override
    @DataClass.Generated.Member
    public int hashCode() {
        // You can override field hashCode logic by defining methods like:
        // int fieldNameHashCode() { ... }

        int _hash = 1;
        _hash = 31 * _hash + java.util.Objects.hashCode(mDomains);
        return _hash;
    }

    @Override
    @DataClass.Generated.Member
    public void writeToParcel(@NonNull Parcel dest, int flags) {
        // You can override field parcelling by defining methods like:
        // void parcelFieldName(Parcel dest, int flags) { ... }

        parcelDomains(dest, flags);
    }

    @Override
    @DataClass.Generated.Member
    public int describeContents() { return 0; }

    /** @hide */
    @SuppressWarnings({"unchecked", "RedundantCast"})
    @DataClass.Generated.Member
    protected DomainSet(@NonNull Parcel in) {
        // You can override field unparcelling by defining methods like:
        // static FieldType unparcelFieldName(Parcel in) { ... }

        Set<String> domains = unparcelDomains(in);

        this.mDomains = domains;
        com.android.internal.util.AnnotationValidations.validate(
                NonNull.class, null, mDomains);

        // onConstructed(); // You can define this method to get a callback
    }

    @DataClass.Generated.Member
    public static final @NonNull Parcelable.Creator<DomainSet> CREATOR
            = new Parcelable.Creator<DomainSet>() {
        @Override
        public DomainSet[] newArray(int size) {
            return new DomainSet[size];
        }

        @Override
        public DomainSet createFromParcel(@NonNull Parcel in) {
            return new DomainSet(in);
        }
    };

    @DataClass.Generated(
            time = 1613169242020L,
            codegenVersion = "1.0.22",
            sourceFile = "frameworks/base/core/java/android/content/pm/verify/domain/DomainSet.java",
            inputSignatures = "private final @android.annotation.NonNull java.util.Set<java.lang.String> mDomains\nprivate  void parcelDomains(android.os.Parcel,int)\nprivate  java.util.Set<java.lang.String> unparcelDomains(android.os.Parcel)\nclass DomainSet extends java.lang.Object implements [android.os.Parcelable]\n@com.android.internal.util.DataClass(genParcelable=true, genAidl=true, genEqualsHashCode=true)")
    @Deprecated
    private void __metadata() {}


    //@formatter:on
    // End of generated code

}